#include "packet.h"

#include <cstdlib>
#include <cstring>  //  for memset...


bool InitializeBuffer(Buffer &buffer, size_t size)
{
    buffer.index = 0;
    buffer.size = size;
    buffer.data = (uint8_t*) malloc(size);

    return buffer.data != NULL;
}

void ClearBuffer(Buffer &buffer)
{
    buffer.index = 0;
    memset(buffer.data, 0, buffer.size);
}

void WriteInteger( Buffer & buffer, uint32_t value )
{
    *((uint32_t*)(buffer.data+buffer.index)) = value; 
    buffer.index += 4;
}

void WriteShort( Buffer & buffer, uint16_t value )
{
    *((uint16_t*)(buffer.data+buffer.index)) = value; 
    buffer.index += 2;
}

void WriteByte( Buffer & buffer, uint8_t value )
{
    *((uint8_t*)(buffer.data+buffer.index)) = value; 
    buffer.index += 1;
}

void WriteString(Buffer &buffer, std::string s)
{
    size_t len = s.length();
    for (size_t i = 0; i < len; ++i)
    {
        buffer.data[i+buffer.index] = s[i];
    }
    buffer.index += len;
}


uint32_t ReadInteger( Buffer & buffer )
{
    uint32_t value;
    value = *((uint32_t*)(buffer.data+buffer.index));
    buffer.index += 4;
    return value;
}

uint16_t ReadShort( Buffer & buffer )
{
    uint16_t value;
    value = *((uint16_t*)(buffer.data+buffer.index));
    buffer.index += 2;
    return value;
}

uint8_t ReadByte( Buffer & buffer )
{
    uint8_t value;
    value = *((uint8_t*)(buffer.data+buffer.index));
    buffer.index += 1;
    return value;
}

//  TODO: ReadString is not efficient, although should not matter (for now)
std::string ReadString(Buffer &buffer, size_t len)
{
    std::string value = "";
    for (size_t i = 0; i < len; ++i)
    {
        value += buffer.data[i+buffer.index];
    }
    buffer.index += value.length();

    return value;
}





PacketLobbyEnter::PacketLobbyEnter()
{
    this->packetType = packetType;
}

PacketLobbyEnter::PacketLobbyEnter(std::string name)
{
    //this->packetType = packetType;
    this->packetType = PACKET_TYPE_LOBBY_ENTER;
    this->playerName = name;
    this->playerNameLen = name.length();
}


void PacketLobbyEnter::Write(Buffer &buffer)
{
    WriteShort(buffer, playerNameLen);
    WriteString(buffer, playerName);
}

void PacketLobbyEnter::Read(Buffer &buffer)
{
    playerNameLen = ReadShort(buffer);
    playerName = ReadString(buffer, playerNameLen);
}

uint16_t PacketLobbyEnter::GetBufferSize()
{
    return sizeof(this->playerNameLen) + this->playerNameLen; 
}









PacketDirection::PacketDirection()
{
    this->packetType = PACKET_TYPE_DIRECTION;
}

PacketDirection::PacketDirection(uint8_t playerId, uint8_t direction, uint32_t lockstepTurn)
{
    this->packetType = PACKET_TYPE_DIRECTION;
    this->playerId = playerId;
    this->direction = direction;
    this->lockstepTurn = lockstepTurn;
}

void PacketDirection::Write(Buffer &buffer)
{
    WriteByte(buffer, playerId);
    WriteByte(buffer, direction);
    WriteInteger(buffer, lockstepTurn);
}

void PacketDirection::Read(Buffer &buffer)
{
    playerId = ReadByte(buffer);
    direction = ReadByte(buffer);
    lockstepTurn = ReadInteger(buffer);
}

uint16_t PacketDirection::GetBufferSize()
{
    return sizeof(playerId) + sizeof(direction) + sizeof(lockstepTurn);
}














PacketDoNothing::PacketDoNothing()
{
    this->packetType = PACKET_TYPE_DO_NOTHING;
}

PacketDoNothing::PacketDoNothing(uint8_t playerId, uint32_t lockstepTurn)
{
    this->packetType = PACKET_TYPE_DO_NOTHING;
    this->playerId = playerId;
    this->lockstepTurn = lockstepTurn;
}

void PacketDoNothing::Write(Buffer &buffer)
{
    WriteByte(buffer, playerId);
    WriteInteger(buffer, lockstepTurn);
}

void PacketDoNothing::Read(Buffer &buffer)
{
    playerId = ReadByte(buffer);
    lockstepTurn = ReadInteger(buffer);
}

uint16_t PacketDoNothing::GetBufferSize()
{
    return sizeof(playerId) + sizeof(lockstepTurn);
}



















PacketActions::PacketActions()
{
    this->packetType = PACKET_TYPE_ACTIONS;
}

PacketActions::PacketActions(std::vector<uint8_t>* actionTypes, std::vector<Packet*>* actions, uint32_t lockstepTurn)
{
    this->packetType = PACKET_TYPE_ACTIONS;

    this->actionCount = (*actions).size();  //  sanity check actions.size == actionTypes.size ?
    
    this->actionTypes = actionTypes;
    this->actions = actions;

    uint16_t totalSize = 0;
    for (size_t i = 0; i < actionCount; ++i)
    {
        totalSize += (*actions)[i]->GetBufferSize();
    }
    this->bufferSize = totalSize;

    
    this->lockstepTurn = lockstepTurn;
}

PacketActions::PacketActions(std::vector<Packet*>* actions, uint32_t lockstepTurn)
{
    this->packetType = PACKET_TYPE_ACTIONS;

    this->actionCount = (*actions).size();  //  sanity check actions.size == actionTypes.size ?
    //std::cerr << "beforereferences\n";
    //std::cerr << "act count:" << (int)this->actionCount << std::endl;
    this->actionTypes = NULL;
    this->actions = actions;

    uint16_t totalSize = 0;
    //std::cerr << "beforeloop\n";
    for (size_t i = 0; i < actionCount; ++i)
    {
        //std::cerr << "iter " << i << " packet type:" << (int) ((*actions)[i]->GetPacketType()) << std::endl;
        totalSize += (*actions)[i]->GetBufferSize();
    }
    //std::cerr << "afterloop\n";

    //  add size of action types to total size
    totalSize += actionCount;

    //  add one byte for actionCount
    totalSize += 1;

    //  add 4 bytes for lockstep turn
    totalSize += 4;


    this->bufferSize = totalSize;

    
    this->lockstepTurn = lockstepTurn;
}

void PacketActions::Write(Buffer &buffer)
{
    WriteByte(buffer, actionCount);
    for (size_t i = 0; i < actionCount; ++i)
    {
        if (actionTypes != NULL)
            WriteByte(buffer, (*actionTypes)[i]);
        else
            WriteByte(buffer, (*actions)[i]->GetPacketType() ) ;
        (*actions)[i]->Write(buffer);
    }
    WriteInteger(buffer, lockstepTurn);
}

void PacketActions::Read(Buffer &buffer)
{
    actionCount = ReadByte(buffer);

    actionTypes = new std::vector<uint8_t>();
    actions = new std::vector<Packet*>();
    

    uint16_t totalSize = 0;
    for (uint8_t i = 0; i < actionCount; ++i)
    {
        actionTypes->push_back(ReadByte(buffer));

        if (actionTypes->back() == PACKET_TYPE_DIRECTION)
        {
            PacketDirection* packet = new PacketDirection();
            packet->Read(buffer);
            actions->push_back(packet);
            totalSize += packet->GetBufferSize();
        }
        else if (actionTypes->back() == PACKET_TYPE_DO_NOTHING)
        {
            PacketDoNothing* packet = new PacketDoNothing();
            packet->Read(buffer);
            actions->push_back(packet);
            totalSize += packet->GetBufferSize();
        }
    }
    this->bufferSize = totalSize;
    
    lockstepTurn = ReadInteger(buffer);
}

uint16_t PacketActions::GetBufferSize()
{
    return this->bufferSize;
}
























PacketPlayerList::PacketPlayerList()
{
    this->packetType = PACKET_TYPE_PLAYER_LIST;
}

PacketPlayerList::PacketPlayerList(uint8_t playerCount, std::vector<uint8_t>playerIds, uint8_t playerId)
{
    this->playerCount = playerCount;
    for (size_t i = 0; i < playerCount; ++i)//maybe sanity check for playerCount == size of vector
    {
        this->playerIds.push_back(playerIds[i]);
    }
    this->playerId = playerId;


    this->packetType = PACKET_TYPE_PLAYER_LIST;
}

PacketPlayerList::PacketPlayerList(uint8_t playerCount, std::vector<uint8_t>playerIds, uint8_t playerId, std::vector<std::string> playerNames)
{
    this->playerCount = playerCount;
    for (size_t i = 0; i < playerCount; ++i)//maybe sanity check for playerCount == size of vector
    {
        this->playerIds.push_back(playerIds[i]);
        this->playerNames.push_back(playerNames[i]);
    }
    this->playerId = playerId;


    this->packetType = PACKET_TYPE_PLAYER_LIST;
}

void PacketPlayerList::Write(Buffer &buffer)
{
    WriteByte(buffer, playerCount);
    for (size_t i = 0; i < playerCount; ++i)//maybe sanity check for playerCount == size of vector
    {
        WriteByte(buffer, playerIds[i]);
        WriteShort(buffer, playerNames[i].length());
        WriteString(buffer, playerNames[i]);
    }
    WriteByte(buffer, playerId);
}

void PacketPlayerList::Read(Buffer &buffer)
{
    playerCount = ReadByte(buffer);

    for (size_t i = 0; i < playerCount; ++i)//maybe sanity check for playerCount == size of vector
    {
        playerIds.push_back(ReadByte(buffer));
        uint16_t playerNameLength = ReadShort(buffer);
        playerNames.push_back(ReadString(buffer, playerNameLength));
    }

    playerId = ReadByte(buffer);
}

uint16_t PacketPlayerList::GetBufferSize()
{
    uint16_t sumNameLengths = 0;
    for (size_t i = 0; i < playerCount; ++i)
    {
        sumNameLengths += playerNames[i].length();
    }
    return sizeof(playerCount) + playerCount*sizeof(playerId) + sizeof(playerId)
            +playerCount*sizeof(uint16_t) + sumNameLengths;
}







































PacketClientReady::PacketClientReady()
{
    this->packetType = PACKET_TYPE_CLIENT_READY;
    ready = 1;
}

void PacketClientReady::Write(Buffer &buffer)
{
    WriteByte(buffer, ready);
}

void PacketClientReady::Read(Buffer &buffer)
{
    ready = ReadByte(buffer);
}

uint16_t PacketClientReady::GetBufferSize()
{
    return sizeof(ready);
}
















PacketClientStart::PacketClientStart()
{
    this->packetType = PACKET_TYPE_CLIENT_START;
    start = 1;
}

void PacketClientStart::Write(Buffer &buffer)
{
    WriteByte(buffer, start);
}

void PacketClientStart::Read(Buffer &buffer)
{
    start = ReadByte(buffer);
}

uint16_t PacketClientStart::GetBufferSize()
{
    return sizeof(start);
}