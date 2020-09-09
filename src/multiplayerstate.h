#ifndef MULTIPLAYERSTATE_H
#define MULTIPLAYERSTATE_H

#include <map>
#include <vector>
#include <cstdint> //  for uint types


#include <allegro5/allegro.h>
#include "allegro5/allegro_image.h"
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "gamestate.h"
#include "menubutton.h"
#include "socket.h"


class MultiPlayerState : public GameState
{
public:
	void Init();
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents(GameEngine* game);
	void Update(GameEngine* game);
	void Draw(GameEngine* game);

	static MultiPlayerState* Instance() 
    {
		return &m_MenuState;
	}

protected:
	MultiPlayerState() { }

private:
	static MultiPlayerState m_MenuState;

	
	bool InitHost();
	bool InitClient();

	void Draw2();



	bool renderThreadWorking;
	static void* RenderThread(ALLEGRO_THREAD* thread, void* arg);
	ALLEGRO_THREAD* renderThread;


	bool logicThreadWorking;
	static void* LogicThread(ALLEGRO_THREAD* thread, void* arg);
	ALLEGRO_THREAD* logicThread;
	static bool LogicThreadLockstepReceive(MultiPlayerState* cptr);
	static bool LogicThreadLockstepBroadcast(MultiPlayerState* cptr);


	bool receiveThreadWorking;
	ALLEGRO_THREAD* receiveThread;
	static void* ReceiveThread(ALLEGRO_THREAD* thread, void* arg);


	static bool HostBroadcastMessages(uint32_t packetTurn, void* arg);



	//  for storing key=value pairs inside config file
	std::map<std::string, std::string> configMap;

	std::vector<ALLEGRO_COLOR> playerColors;

	std::vector<int> playerStartX;
	std::vector<int> playerStartY;
	std::vector<uint8_t> playerStartDir;
	
	std::vector<int> playerX;
	std::vector<int> playerY;
	std::vector<uint8_t> playerDir;
	std::vector< std::vector<int> > playerAllX;
	std::vector< std::vector<int> > playerAllY;
	std::vector< std::vector<uint8_t> > playerAllDir;
	
	
	//  sound effects
	ALLEGRO_SAMPLE* tickSound;
	ALLEGRO_SAMPLE* crashSound;


	//  is this the right place?
	bool pressedKeys[ALLEGRO_KEY_MAX];


	 
	uint8_t playerId;
	int playerCount;
	


	//  isHost, hostId/hostPlayer/hostPlayerId

	uint32_t logicFrameCounter;
	uint32_t renderFrameCounter;
	uint32_t currentLockstepTurn;
	std::vector<uint64_t> lockstepTurnFinishTimesMilliseconds;




	
	/*
	//	network variables
	*/
	int hostPort;
	int numClients;
	bool isHost;
	std::vector< std::vector<Packet*> > turnMessages;
	

	//  server network variables
	Socket listenerSocket;
	std::vector<Socket> acceptSockets;  //  name may be changed to clientSockets
	uint8_t nextPlayerId;
	std::vector< std::vector<Packet*> > clientMessages;
	std::vector<Packet*> selfMessages;
	int lastSentTurn;

	//  client network variables
	Socket clientSocket;
	std::vector< std::vector<Packet*> > messagesFromHost;
	int lastReceivedTurn;

};

#endif
