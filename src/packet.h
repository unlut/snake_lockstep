#ifndef PACKET_H
#define PACKET_H

#include <iostream>
#include <cstdint>
#include <vector>

#define CURRENT_PROTOCOL_NUMBER 0




struct Buffer
{
    uint8_t * data;     // pointer to buffer data
    int size;           // size of buffer data (bytes)
    int index;          // index of next byte to be read/written
};


bool InitializeBuffer(Buffer &buffer, size_t size);
void ClearBuffer(Buffer &buffer);


void WriteInteger( Buffer & buffer, uint32_t value );
void WriteShort( Buffer & buffer, uint16_t value );
void WriteByte( Buffer & buffer, uint8_t value );
void WriteString( Buffer &buffer, std::string s);

uint32_t ReadInteger( Buffer & buffer );
uint16_t ReadShort( Buffer & buffer );
uint8_t ReadByte( Buffer & buffer );
std::string ReadString (Buffer &buffer);


enum PACKET_TYPE
{
    PACKET_TYPE_LOBBY_ENTER,
    PACKET_TYPE_DIRECTION,
    PACKET_TYPE_PLAYER_LIST,
    PACKET_TYPE_CLIENT_READY,
    PACKET_TYPE_CLIENT_START,
    PACKET_TYPE_DO_NOTHING,
    PACKET_TYPE_ACTIONS,
};


class Packet
{
private:
   
protected:
    static const uint16_t protocolNumber {CURRENT_PROTOCOL_NUMBER};  //  starts from 0, incremented with each update
    uint32_t sequenceId;      //  number of messages send in THIS connection(or socket for TCP connections), starts from 0 (uint16 if need memory so much)
    uint8_t packetType;
    uint16_t bufferSize;

public:

    virtual void Write(Buffer &buffer) = 0;
    virtual void Read(Buffer &buffer) = 0;


    //  this one is packet specific
    virtual uint16_t GetBufferSize() = 0;

    void WriteHeader(Buffer &buffer) 
    { 
        WriteShort(buffer, protocolNumber);
        WriteInteger(buffer, sequenceId);
        WriteByte(buffer, packetType); 
        WriteShort(buffer, GetBufferSize());
    }

    void ReadHeader(Buffer &buffer)
    {
        //protocolNumber = ReadShort(buffer);
        ReadShort(buffer);  //  skip protocolnumber, no need to assign
        sequenceId = ReadInteger(buffer);
        packetType = ReadByte(buffer);
        bufferSize = ReadShort(buffer);
    }

    static uint16_t GetPacketHeaderSize() 
    { 
        return sizeof(protocolNumber) + sizeof(sequenceId) + sizeof(packetType) + sizeof(bufferSize); 
    }

    void SetSequenceId(uint32_t sequenceId)
    {
        this->sequenceId = sequenceId;
    }

    void SetPacketType(uint8_t packetType)
    {
        this->packetType = packetType;
    }

    void SetBufferSize(uint16_t bufferSize)
    {
        this->bufferSize = bufferSize;
    }

    //  getters
    uint16_t GetProtocolNumber() { return protocolNumber; }
    uint32_t GetSequenceId() { return sequenceId; }
    uint8_t GetPacketType() { return packetType; }
};



class PacketLobbyEnter : public Packet
{
private:

public:
    uint16_t playerNameLen;
    std::string playerName;

    PacketLobbyEnter();
    PacketLobbyEnter(std::string name);

    void Write(Buffer &buffer);
    void Read(Buffer &buffer);

    uint16_t GetBufferSize();
};

class PacketDirection : public Packet
{
private:

public:
    uint8_t playerId;
    uint8_t direction;
    uint32_t lockstepTurn;

    PacketDirection();
    PacketDirection(uint8_t playerId, uint8_t direction, uint32_t lockstepTurn);

    void Write(Buffer &buffer);
    void Read(Buffer &buffer);

    uint16_t GetBufferSize();
};


class PacketDoNothing : public Packet
{
private:

public:
    uint8_t playerId;
    uint32_t lockstepTurn;

    PacketDoNothing();
    PacketDoNothing(uint8_t playerId, uint32_t lockstepTurn);

    void Write(Buffer &buffer);
    void Read(Buffer &buffer);

    uint16_t GetBufferSize();
};


class PacketActions : public Packet
{
private:

public:
    
    uint8_t actionCount;
    uint32_t lockstepTurn;

    std::vector<uint8_t>* actionTypes;
    std::vector<Packet*>* actions;

    PacketActions();
    PacketActions(std::vector<uint8_t>* actionTypes, std::vector<Packet*>* actions, uint32_t lockstepTurn);
    PacketActions(std::vector<Packet*>* actions, uint32_t lockstepTurn);
    

    void Write(Buffer &buffer);
    void Read(Buffer &buffer);

    uint16_t GetBufferSize();
};




class PacketPlayerList : public Packet
{
private:

public:
    //  number of players in the game
    uint8_t playerCount;

    //  ids of players in the game
    std::vector<uint8_t> playerIds;

    //  names of players in the game
    std::vector<std::string> playerNames;

    //  id of player receiving this message
    uint8_t playerId;  //  maybe change this variable name to whichPlayer
    

    PacketPlayerList();
    PacketPlayerList(uint8_t playerCount, std::vector<uint8_t>playerIds, uint8_t playerId);
    PacketPlayerList(uint8_t playerCount, std::vector<uint8_t>playerIds, uint8_t playerId, std::vector<std::string> playerNames);

    void Write(Buffer &buffer);
    void Read(Buffer &buffer);

    uint16_t GetBufferSize();
};

class PacketClientReady : public Packet
{
private:

public:
    uint8_t ready;

    PacketClientReady();

    void Write(Buffer &buffer);
    void Read(Buffer &buffer);

    uint16_t GetBufferSize();
};


class PacketClientStart : public Packet
{
private:

public:
    uint8_t start;

    PacketClientStart();

    void Write(Buffer &buffer);
    void Read(Buffer &buffer);

    uint16_t GetBufferSize();
};




#endif