#ifndef SOCKET_H
#define SOCKET_H

#include "packet.h"


/*
UNUTULMAMASI GEREKENLER LISTESI

1- Bir socket 2 sekilde olusabilir, ya sifirdan socket olusturarak yada
accept ile

2- Socketlerin nonblocking ve nodelay yapilmasi gerek


EAGAIN - Resource temporarily unavailable (usually due to nonblocking mode)


*/


class Socket
{
private:
    int handle;

    unsigned short port;

    unsigned char a, b, c, d;  //  4 parts of an IP4 address
    unsigned int address;

    bool isTCP;
    uint32_t currentSequenceNumber;

    
    //  variables to fight against TCP fragmentation!!!
    Buffer receiveBuffer;
    uint32_t currentReceivedCount;
    uint32_t remainingReceiveCount;

    uint16_t currentProtocolNumber;
    uint32_t currentSequenceId;
    uint8_t currentPacketType;
    uint16_t currentPacketSize;



public:
    Socket();
    bool Init();
    

    

    bool SetNonblocking(bool isNonBlocking=true);//  switching between blocking and nonblocking is not tested!
    bool SetNoDelay(bool isNoDelay=true);
    
    //  Client functions
    bool Connect(const char* ipString, unsigned short port);
    

    //  Server functions:
    bool Bind(unsigned short port);
    int Listen(int backlog);  //  no error checking in listen?!
    bool Accept(Socket &socket);




    void Close();

    bool IsOpen() const;



    bool Send(const void* data, int size);
    bool Send(Packet &packet);

    int Receive(void* data, size_t size);
    Packet* Receive();


    //  waits until there is data to read in the socket
    int ReceivePoll(int timeout);




    bool IsInitialized();


    //  required for windows platform
    static bool InitializeSockets();
    static void ShutdownSockets();
};


#endif