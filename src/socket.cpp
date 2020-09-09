#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "socket.h"

// platform detection
#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif


//  include required header(s)
#if PLATFORM == PLATFORM_WINDOWS

    #include <winsock2.h>

    //  in windows you also need to link winsock library!

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <fcntl.h>
    #include <arpa/inet.h>
    #include <netinet/tcp.h>  //  FOR TCP NODELAY
    #include <sys/poll.h>


    #include <errno.h>
    #include <string.h>

#endif



Socket::Socket()
{
    this->handle = -1;
    this->isTCP = false;  //  at construction time, protocol is not determined yet
    this->currentSequenceNumber = 0;
    this->currentReceivedCount = 0;
    this->remainingReceiveCount = 0;

    InitializeBuffer(this->receiveBuffer, 2000);
}

bool Socket::IsInitialized()
{
    return this->handle != -1;
}

bool Socket::Init()
{
    int handle = socket( AF_INET, 
                         SOCK_STREAM, 
                         IPPROTO_TCP );

    if ( handle <= 0 )
    {
        printf( "failed to create socket\n" );
        return false;
    }

    //  since we only use TCP sockets for now, set this to true
    this->isTCP = true;

    


    this->handle = handle;

    
    //return this->SetNoDelay(true);

    return true;
}


bool Socket::Bind(unsigned short port)
{
    sockaddr_in address;
    //memset(&address, '0', sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  //  htonl(INADDR_ANY);
    address.sin_port = htons( (unsigned short) port );

    if ( bind( handle, 
               (const sockaddr*) &address, 
               sizeof(sockaddr_in) ) < 0 )
    {
        printf( "failed to bind socket\n" );
        return false;
    }


    return true;
}



bool Socket::SetNonblocking(bool isNonBlocking)
{
    //  Put socket in non-blocking mode
    #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

        int nonBlocking = isNonBlocking ? 1 : 0;
        if ( fcntl( handle, 
                    F_SETFL, 
                    O_NONBLOCK, 
                    nonBlocking ) == -1 )
        {
            printf( "failed to set non-blocking\n" );
            return false;
        }

    #elif PLATFORM == PLATFORM_WINDOWS

        DWORD nonBlocking = isNonBlocking ? 1 : 0;
        if ( ioctlsocket( handle, 
                          FIONBIO, 
                          &nonBlocking ) != 0 )
        {
            printf( "failed to set non-blocking\n" );
            return false;
        }

    #endif

    return true;
}

bool Socket::SetNoDelay(bool isNoDelay)
{
     #if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

        //  set tcp nodelay
        int yes = isNoDelay ? 1 : 0;
        int retval = setsockopt(handle,
                                IPPROTO_TCP,
                                TCP_NODELAY,
                                (char *) &yes, 
                                sizeof(int));    // 1 - on, 0 - off
        if (retval < 0)
        {
            printf("Unable to set TCP NODELAY for socket!\n");
            return false;
        }

    #elif PLATFORM == PLATFORM_WINDOWS

        printf("INVALID FOR WINDOWS - NoDelay\n");
        hataverensatir

    #endif

    return true;
}


bool Socket::Connect(const char* ipString, unsigned short port)
{
    struct sockaddr_in server;

    server.sin_addr.s_addr = inet_addr(ipString);
	server.sin_family = AF_INET;
	server.sin_port = htons( port );

	//Connect to remote server
	if (connect(handle , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
        printf("Connect error\n");
		return false;
	}

    return true;
}

int Socket::Listen(int backlog)
{
    int val = listen(handle, backlog);
    //  no error checking in listen?!
    return val;
}

bool Socket::Accept(Socket &socket)
{
    struct sockaddr_in client;
    int c = sizeof(struct sockaddr_in);
    int new_socket = accept(handle, (struct sockaddr *)&client, (socklen_t*)&c);

    #if PLATFORM == PLATFORM_WINDOWS
        if (new_socket == INVALID_SOCKET)
        {
            return false;
        }
    #elif PLATFORM == PLATFORM_UNIX || PLATFORM == PLATFORM_MAC
        if (new_socket < 0)
        {
            return false;
        }
    #endif

    
    //  if listening socket is TCP, make new socket TCP too
    if (this->isTCP)
    {
        socket.isTCP = true;
    }

    //
    //  fill values of socket class here before return 
    //
    socket.handle = new_socket;

    char *client_ip = NULL;
    if (client.sin_family == AF_INET)
    {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&client;
        client_ip = (char*) malloc(INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(addr_in->sin_addr), client_ip, INET_ADDRSTRLEN);
    }

    /*  IPv6 part removed for now
    else if (client.sa_family == AF_INET6)
    {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)client;
        client_ip = malloc(INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), client_ip, INET6_ADDRSTRLEN);
    }
    */

    printf("Client IP address: %s\n", client_ip);
    std::string ipStdString = std::string(client_ip);
    std::string delim = ".";
    std::string token;
    std::vector<unsigned char> nums;

    size_t start = 0;
    size_t end = ipStdString.find(delim);
    while (end != std::string::npos)
    {
        token = ipStdString.substr(start, end-start);
        nums.push_back(std::stoi(token));
        std::cout << token << std::endl;
        start = end + delim.length();
        end = ipStdString.find(delim, start);
    }
    token = ipStdString.substr(start, end);
    nums.push_back(std::stoi(token));
    std::cout << token << std::endl;


    if (nums.size() != 4)
    {
        std::cerr << "Socket::Accept() --- Not an IP4 address or parsing error occured!" << std::endl;
    }
    socket.a = nums[0];
    socket.b = nums[1];
    socket.c = nums[2];
    socket.d = nums[3];

    

    unsigned short client_port = ntohs(client.sin_port);
    socket.port = client_port;
    printf("Client Port:%d\n", client_port);


    //  set nodelay
    return socket.SetNoDelay(true);

    //return true;
}

/*
struct Buffer
{
    uint8_t * data;     // pointer to buffer data
    int size;           // size of buffer data (bytes)
    int index;          // index of next byte to be read/written
};
*/

bool Socket::Send(Packet& packet)
{
    Buffer b;
    b.size = packet.GetBufferSize() + packet.GetPacketHeaderSize();  
    b.index = 0;
    b.data = (uint8_t*) malloc(b.size);

    packet.SetSequenceId(this->currentSequenceNumber);
    currentSequenceNumber++;

    //printf("Send determined size:%d\n", b.size);
    //printf("ID of currently sended packet:%d\n", currentSequenceNumber-1);

    packet.WriteHeader(b);
    packet.Write(b);

    int retval = send(this->handle, b.data, b.size, 0);

    //  does this case happen?
    //maybe need fix
    if (retval < b.size)
    {
        fprintf(stderr, "Socket::Send() --- Unable to send all data!\n");
    }

    #if PLATFORM == PLATFORM_UNIX || PLATFORM == PLATFORM_MAC
        return retval > 0;
    #elif PLATFORM == PLATFORM_WINDOWS
        hataverensatir
    #endif


    return true;
}

/*
bool Socket::Receive(void* data, size_t size)
{
    int retval = recv(this->handle, data, size, 0);

    
    #if PLATFORM == PLATFORM_UNIX
        if (retval > 0)
            return true;
        
        else if (retval == 0)
        {
            //  end or empty?
            ;
            return false;
        }
        else
        {
            return false;
        }
    #elif PLATFORM == PLATFORM_WINDOWS
        hataverensatir
    #endif

    return false;
}
*/

int Socket::Receive(void* data, size_t size)
{
    int retval = recv(this->handle, data, size, 0);

    
    #if PLATFORM == PLATFORM_UNIX
        return retval;
    #elif PLATFORM == PLATFORM_WINDOWS
        hataverensatir
    #endif

    return false;
}

Packet* Socket::Receive()
{
    uint16_t headerSize = Packet::GetPacketHeaderSize();  //  maybe calculate this earlier once

    if (this->currentReceivedCount == 0)
    {
        //  start reading a new message
        int retval = recv(this->handle, this->receiveBuffer.data, headerSize, 0);

        if (retval > 0)
        {
            //  a message is arrived
            //attempt to read header
            if (retval == headerSize)
            {
                currentProtocolNumber = ReadShort(this->receiveBuffer);
                currentSequenceId = ReadInteger(this->receiveBuffer);
                currentPacketType = ReadByte(this->receiveBuffer);
                currentPacketSize = ReadShort(this->receiveBuffer);

                //  read rest of the message
                retval = recv(this->handle, this->receiveBuffer.data+headerSize, currentPacketSize, 0);

                if (currentPacketSize == retval)
                {
                    //  complete package is read via recv, no more things need to be done
                    //identify, form and return the package
                    Packet* readPacket = NULL;

                    if (currentPacketType == PACKET_TYPE_LOBBY_ENTER)
                    {
                        readPacket = new PacketLobbyEnter();
                        readPacket->Read(receiveBuffer); 
                    }
                    else if (currentPacketType == PACKET_TYPE_DIRECTION)
                    {
                        readPacket = new PacketDirection();
                        readPacket->Read(receiveBuffer);
                    }
                    else if (currentPacketType == PACKET_TYPE_PLAYER_LIST)
                    {
                        readPacket = new PacketPlayerList();
                        readPacket->Read(receiveBuffer);
                    }
                    else if (currentPacketType == PACKET_TYPE_CLIENT_READY)
                    {
                        readPacket = new PacketClientReady();
                        readPacket->Read(receiveBuffer);
                    }
                    else if (currentPacketType == PACKET_TYPE_CLIENT_START)
                    {
                        readPacket = new PacketClientStart();
                        readPacket->Read(receiveBuffer);
                    }
                    else if (currentPacketType == PACKET_TYPE_DO_NOTHING)
                    {
                        readPacket = new PacketDoNothing();
                        readPacket->Read(receiveBuffer);
                    }
                    else if (currentPacketType == PACKET_TYPE_ACTIONS)
                    {
                        readPacket = new PacketActions();
                        readPacket->Read(receiveBuffer);
                    }

                    //  clear receive buffer
                    ClearBuffer(this->receiveBuffer);

                    //  fill in header values
                    //TODO: Maybe a better way of doing this?
                    readPacket->SetSequenceId(currentSequenceId);
                    readPacket->SetPacketType(currentPacketType);
                    readPacket->SetBufferSize(currentPacketSize);
                   

                    return readPacket;
                }
                else if (currentPacketSize > retval)
                {
                    //  some part of the package is missing
                    //for now, store the amount read
                    this->currentReceivedCount += headerSize + retval;
                    this->remainingReceiveCount = currentPacketSize - retval;

                    std::cerr << "Socket::Receive() - Some part of the message is arrived" << std::endl;

                    return NULL;
                }
                else if (currentPacketSize < retval)
                {
                    //  should be impossible?
                    std::cerr << "Socket::Receive() - pkgsize < retval, should be impossible?" << std::endl;
                    return NULL;
                }
            }
            else
            {
                //  data read via recv is too small, it does not even contain the packet header

                //  TODO: for now error message and terminate
                std::cerr << "Socket::Receive() - Message read via recv is too small to contain whole header!" << std::endl;
                return NULL;
            }

        }
        else if (retval == 0)
        {
            //  end of communication message?
            //  TO DO: Handle this
            std::cerr << "Socket::Receive() - 0 byte arrived!" << std::endl;
            exit(0);
        }
        else
        {
            //  negative value, error
            //  TO DO: maybe return a class indicating which error happened
            //TODO: EAGAIN ve EWOULDBLOCK handle olayini biraz dusun
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {

            }
            else
            {
                std::cerr << "Socket::Receive() - Negative value returned!" << std::endl;
                fprintf(stderr, "socket() failed: %s\n", strerror(errno));
            }
            
            return NULL;
        }
        
    }
    else
    {
        //  continue reading a previously started message
        int retval = recv(this->handle, this->receiveBuffer.data + this->currentReceivedCount, this->remainingReceiveCount, 0);

        if (retval == this->remainingReceiveCount)
        {
            //  message read is completed

            //  TO DO: Convert below part into a function, or somehow eliminate duplicate same blocks
            //  complete package is read via recv, no more things need to be done
            //identify, form and return the package
            Packet* readPacket = NULL;

            if (currentPacketType == PACKET_TYPE_LOBBY_ENTER)
            {
                readPacket = new PacketLobbyEnter();
                readPacket->Read(receiveBuffer); 
            }
            else if (currentPacketType == PACKET_TYPE_DIRECTION)
            {
                readPacket = new PacketDirection();
                readPacket->Read(receiveBuffer);
            }
            else if (currentPacketType == PACKET_TYPE_PLAYER_LIST)
            {
                readPacket = new PacketPlayerList();
                readPacket->Read(receiveBuffer);
            }
            else if (currentPacketType == PACKET_TYPE_CLIENT_READY)
            {
                readPacket = new PacketClientReady();
                readPacket->Read(receiveBuffer);
            }
            else if (currentPacketType == PACKET_TYPE_CLIENT_START)
            {
                readPacket = new PacketClientStart();
                readPacket->Read(receiveBuffer);
            }
            else if (currentPacketType == PACKET_TYPE_DO_NOTHING)
            {
                readPacket = new PacketDoNothing();
                readPacket->Read(receiveBuffer);
            }
            else if (currentPacketType == PACKET_TYPE_ACTIONS)
            {
                readPacket = new PacketActions();
                readPacket->Read(receiveBuffer);
            }


            //  clear receive buffer
            ClearBuffer(this->receiveBuffer);

            //  fill in header values
            //TODO: Maybe a better way of doing this?
            readPacket->SetSequenceId(currentSequenceId);
            readPacket->SetPacketType(currentPacketType);
            readPacket->SetBufferSize(currentPacketSize);

            return readPacket;

        }
        else if (retval < this->remainingReceiveCount)
        {
            //  there is still bytes left to read
            this->currentReceivedCount += retval;
            this->remainingReceiveCount -= retval;

            std::cerr << "Socket::Receive() - Some part of the message is arrived again" << std::endl;
        }

    }
    return NULL;
}



/*
    Linux example taken from:
https://www.ibm.com/support/knowledgecenter/ssw_ibm_i_73/rzab6/poll.htm
*/
int Socket::ReceivePoll(int timeout)
{
    int nfds = 1;

    int retval;

    
    #if PLATFORM == UNIX || PLATFORM == MAC
        struct pollfd  fds[1];

        //  initialize pollfd structure
        memset(fds, 0, sizeof(fds));
        fds[0].fd = this->handle;
        fds[0].events = POLLIN;  //  only wait for readable event

        retval = poll(fds, nfds, timeout);

        if (retval < 0)
        {
            //  poll failed
            std::cerr << "Socket::ReceivePoll() --- poll failed" << std::endl;
        }
        else if (retval == 0)
        {
            //  timeout
            std::cerr << "Socket::ReeivePoll() --- timeout" << std::endl; 
        }
        else
        {
            //  (if multiple sockets, at least one of the sockets is ready to be read)

            if (fds[0].revents != 0 
                && fds[0].revents == POLLIN)
            {
                ;
            }
            else
            {
                std::cerr << "Socket::ReceivePoll() --- unexpect result from poll" << std::endl;
                retval = -150;
            }
        }
    #elif PLATFORM == PLATFORM_WINDOWS
        hataverensatir
    #endif

    return retval;
}


bool Socket::InitializeSockets()
{
    #if PLATFORM == PLATFORM_WINDOWS
        WSADATA WsaData;
        return WSAStartup( MAKEWORD(2,2), 
                            &WsaData ) 
            == NO_ERROR;
    #else
        return true;
    #endif
}

void Socket::ShutdownSockets()
{
    #if PLATFORM == PLATFORM_WINDOWS
           WSACleanup();
    #endif
}