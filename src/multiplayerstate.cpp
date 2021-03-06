
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <algorithm>

#include <chrono>
#include <ctime>


#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>


#include "gameengine.h"
#include "gamestate.h"
#include "introstate.h"
#include "playstate.h"
#include "macros.h"
#include "menustate.h"
#include "menubutton.h"
#include "multiplayerstate.h"


MultiPlayerState MultiPlayerState::m_MenuState;


bool MultiPlayerState::InitHost()
{
	playerId = 0;
	nextPlayerId = 1;

	std::map<int, int> playerIdMap;

	if (listenerSocket.Init() == false)
	{
		std::cerr << "Host --- Unable to initialize listener socket!" << std::endl;
	}

	if (listenerSocket.Bind(hostPort) == false)
	{
		std::cerr << "Host --- Unable to bind listener socket!" << std::endl;
	}

	
	if (listenerSocket.SetNonblocking() == false)
	{
		std::cerr << "Host --- Unable to set listener socket nonblocking!" << std::endl;
	}
	

	
	listenerSocket.Listen(numClients);
	
	//  create N uninitialized sockets for accepted sockets
	for (int i = 0; i < numClients; ++i)
	{
		acceptSockets.push_back(Socket());
	}

	int numAcceptedClients = 0;

	while (numAcceptedClients < numClients)
	{
		std::cerr << "Host --- Waiting for incoming connections..." << std::endl;
		for (int i = 0; i < numClients; ++i)
		{
			if (acceptSockets[i].IsInitialized())
			{
				//  skip these
			}
			else
			{
				bool acceptResult = listenerSocket.Accept(acceptSockets[i]);
				if (acceptResult == false)
				{
					//  need better error printing/handling
				}
				else
				{
					if (acceptSockets[i].SetNonblocking(true) == false)
					{
						std::cerr << "Host --- Unable to set acceptSockets[" << i << "] nonblocking!" << std::endl;
					}
					if (acceptSockets[i].SetNoDelay(true) == false)
					{
						std::cerr << "Host --- Unable to set acceptSockets[" << i << "] nodelay!" << std::endl;	
					}

					//  increment number of accepted clients
					++numAcceptedClients;

					//  players who joined earlier must have a lesser id, regardless of loop iteration counter
					playerIdMap[i] = nextPlayerId;
					++nextPlayerId;
				}
			}
		}
		al_rest(1.0);
	}

	//  perform required swaps in acceptSockets vector
	//so acceptSockets[0] corresponds to first joined player
	//acceptSockets[1] seconds joined player
	//...
	for (int i = 0; i < numAcceptedClients; ++i)
	{
		int desiredPosition = playerIdMap[i] - 1;
		std::iter_swap(acceptSockets.begin()+i, acceptSockets.begin()+desiredPosition);
	}

	std::cerr << "Host --- All " << numAcceptedClients << " clients are connected." << std::endl;

	//  initialize clientNames variable
	clientNames.resize(numAcceptedClients);

	//  wait for an initial message from clients which contains their name
	int numKnownClients = 0;
	std::vector<bool> clientIdentified(numClients);
	while (numKnownClients < numClients)
	{
		std::cerr << "Host --- Waiting for identifying messages..." << std::endl;
		for (int i = 0; i < numClients; ++i)
		{
			if (!clientIdentified[i])
			{
				Packet* receivedPacket = acceptSockets[i].Receive();
				if (receivedPacket == NULL)
				{
					//  better error handling/printing
				}
				else
				{
					uint8_t packetType = receivedPacket->GetPacketType();
					uint16_t packetSize = receivedPacket->GetBufferSize();
					fprintf(stderr, "Packet arrived, protocolId:%d, seqId:%d\n", receivedPacket->GetProtocolNumber(), receivedPacket->GetSequenceId());
					if (packetType == PACKET_TYPE_LOBBY_ENTER)
					{
						fprintf(stderr, "Lobby enter package arrived, size(excluding header):%d\n", packetSize);
						PacketLobbyEnter enteredPackage = *(static_cast<PacketLobbyEnter*> (receivedPacket));
						fprintf(stderr, "Entered player name:%s\n", enteredPackage.playerName.c_str());
						++numKnownClients;
						clientIdentified[i] = true;

						//  save client name
						clientNames[i] = enteredPackage.playerName;
					}
				}
			}
		}
		al_rest(1.0);
	}

	std::cerr << "Host --- All clients are identified." << std::endl;
	std::cerr << "Host --- numKnownClients:" << numKnownClients << std::endl;
	al_rest(2.0);


	//  send each player information about the players in the game
	uint8_t numPlayers = numKnownClients + 1;  //  +1 for host
	std::vector<uint8_t> playerIds;
	for (int i = 0; i < numPlayers; ++i)
	{
		playerIds.push_back(i);
	}
	uint8_t whichPlayer = 0;  //  host is assumed to be 0

	//  initialize playerNames and playerAlive variable
	playerNames.resize(numPlayers);
	playerNames[0] = playerName;
	playerAlive[0] = true;
	for (int i = 0; i < numKnownClients; ++i)
	{
		playerNames[i+1] = clientNames[i];
		playerAlive[i+1] = true;
	}

	for (int i = 0; i < numKnownClients; ++i)
	{
		whichPlayer = playerIdMap[i];

		PacketPlayerList playerList(numPlayers, playerIds, whichPlayer, playerNames);
		acceptSockets[i].Send(playerList);
	}
	this->playerCount = numPlayers;


	int readyClientCount = 0;
	std::vector<bool> clientReady(numClients);
	while (readyClientCount < numKnownClients)
	{
	    std::cerr << "Host --- Waiting 'ready' signal from all clients..." << std::endl;
		for (int i = 0; i < numKnownClients; ++i)
		{
			if (!clientReady[i])
			{
				Packet* receivedPacket = acceptSockets[i].Receive();
				if (receivedPacket == NULL)
				{
					//  better error handling/printing
				}
				else
				{
					uint8_t packetType = receivedPacket->GetPacketType();
					uint16_t packetSize = receivedPacket->GetBufferSize();
					fprintf(stderr, "Packet arrived, protocolId:%d, seqId:%d\n", receivedPacket->GetProtocolNumber(), receivedPacket->GetSequenceId());
					if (packetType == PACKET_TYPE_CLIENT_READY)
					{
						fprintf(stderr, "Lobby enter package arrived, size(excluding header):%d\n", packetSize);
						PacketClientReady enteredPackage = *(static_cast<PacketClientReady*> (receivedPacket));
						++readyClientCount;
						clientReady[i] = true;
					}
				}
			}
		}
		al_rest(1.0);
	}



	//  send game start message to clients
	std::cerr << "Host --- All clients are ready, sending game start countdown signal to clients." << std::endl;
	for (int i = 0; i < readyClientCount; ++i)
	{
		whichPlayer = playerIdMap[i];

		PacketClientStart clientStart;
		acceptSockets[i].Send(clientStart);
	}

	isHost = true;
	lastSentTurn = -1;
}

bool MultiPlayerState::InitClient()
{

	if (clientSocket.Init() == false)
	{
		std::cerr << "Client --- Unable to initialize client socket!" << std::endl;
	}

	//  attempt to connect to host
	if (clientSocket.Connect(configMap["hostip"].c_str(), hostPort) == false)
	{
		//printf("Socket error is:%s\n",strerror(errno));
		std::cerr << "Client --- Unable to connect to server!" << std::endl;
	}

	

	if (clientSocket.SetNoDelay(true) == false)
	{
		std::cerr << "Client --- Unable to set socket TCP NODELAY" << std::endl;
	}

	//  send player name information to host
	PacketLobbyEnter enterPacket(configMap["playername"]);
	clientSocket.Send(enterPacket);


	//  receive information about players in game
	Packet* receivedPacket = clientSocket.Receive();
	if (receivedPacket == NULL)
	{
		//  better error handling/printing
	}
	else
	{
		uint8_t packetType = receivedPacket->GetPacketType();
		uint16_t packetSize = receivedPacket->GetBufferSize();
		fprintf(stderr, "Packet arrived, protocolId:%d, seqId:%d\n", receivedPacket->GetProtocolNumber(), receivedPacket->GetSequenceId());
		if (packetType == PACKET_TYPE_PLAYER_LIST)
		{
			fprintf(stderr, "Player list package arrived, size(excluding header):%d\n", packetSize);
			PacketPlayerList playerList = *(static_cast<PacketPlayerList*> (receivedPacket));
			std::cerr << "Number of players in the game:" << (int)playerList.playerCount << std::endl;
			std::cerr << "I am player " << (int)playerList.playerId << std::endl;
			this->playerCount = playerList.playerCount;
			this->playerId = playerList.playerId;

			//  store player names
			playerNames.resize(this->playerCount);
			for (size_t i = 0; i < this->playerCount; ++i)
			{
				playerNames[i] = playerList.playerNames[i];
				std::cerr << "Name of player " << (int)i << ":" << playerNames[i] << std::endl;
			}

			//  initialize player status
			for (size_t i = 0; i < this->playerCount; ++i)
			{
				playerAlive[i] = true;
			}
		}
	}


	//  send client ready message to host
	PacketClientReady readyPacket;
	clientSocket.Send(readyPacket);



	//  receive start signal from host
	receivedPacket = clientSocket.Receive();
	if (receivedPacket == NULL)
	{
		//  better error handling/printing
	}
	else
	{
		uint8_t packetType = receivedPacket->GetPacketType();
		uint16_t packetSize = receivedPacket->GetBufferSize();
		fprintf(stderr, "Packet arrived, protocolId:%d, seqId:%d\n", receivedPacket->GetProtocolNumber(), receivedPacket->GetSequenceId());
		if (packetType == PACKET_TYPE_CLIENT_START)
		{
			fprintf(stderr, "Client start package arrived, size(excluding header):%d\n", packetSize);
		}
	}


	//  Make client socket nonblocking AFTER connection is established
	//otherwise you get EINPROGRESS during connect
	if (clientSocket.SetNonblocking(true) == false)
	{
		std::cerr << "Client --- Unable to set client socket nonblocking" << std::endl;
	}

	isHost = false;
	lastReceivedTurn = -1;
}

void MultiPlayerState::Init()
{
	drawFPS = 30;
	mainThreadFPS = 30;

	//  initialize frame counters
	logicFrameCounter = 0;
	renderFrameCounter = 0;



	//  initialize key pressed array
	for (int i = 0; i < ALLEGRO_KEY_MAX; ++i)
	{
		pressedKeys[i] = false;
	}


	//  initialize player colors
	std::cerr << "Initializing player colors..." << std::endl;
	playerColors.push_back(COLOR_RED);
	playerColors.push_back(COLOR_BLUE);
	playerColors.push_back(COLOR_TEAL);
	playerColors.push_back(COLOR_PURPLE);


	//  initialize player positions
	std::cerr << "Initializing player positions..." << std::endl;
	playerStartX.push_back(50); playerStartY.push_back(SCREEN_HEIGHT/2);
	playerStartX.push_back(SCREEN_WIDTH - 50); playerStartY.push_back(SCREEN_HEIGHT/2);
	playerStartX.push_back(SCREEN_WIDTH/2); playerStartY.push_back(50);
	playerStartX.push_back(SCREEN_WIDTH/2); playerStartY.push_back(SCREEN_HEIGHT - 50);

	for (int i = 0; i < 4; ++i)
	{
		//  current position
		playerX.push_back(playerStartX[i]);
		playerY.push_back(playerStartY[i]);

		//  position history
		playerAllX.push_back(std::vector<int>());
		playerAllY.push_back(std::vector<int>());
		playerAllX[i].push_back(playerX[i]);
		playerAllY[i].push_back(playerY[i]);
		
	}


	//  initialize player directions
	playerStartDir.push_back(DIRECTION_RIGHT);
	playerStartDir.push_back(DIRECTION_LEFT);
	playerStartDir.push_back(DIRECTION_DOWN);
	playerStartDir.push_back(DIRECTION_UP);
	for (int i = 0; i < 4; ++i)
	{
		playerDir.push_back(playerStartDir[i]);

		//  direction history
		playerAllDir.push_back(std::vector<uint8_t>());
		playerAllDir[i].push_back(playerDir[i]);
	}


	//  initialize player status
	for (int i = 0; i < 4; ++i)
	{
		playerAlive.push_back(false);
	}

	gameFinishSignal = false;
	


	//  initialize sounds
	std::cerr << "Initializing sounds..." << std::endl;
	tickSound = al_load_sample("sounds/BattleNetTick.wav");
	if (tickSound == NULL)
	{
		std::cerr << "Unable to load tickSound!" << std::endl;
	}
	crashSound = al_load_sample("sounds/RockHeavyBashMetal1.wav");
	if (crashSound == NULL)
	{
		std::cerr << "Unable to load crashSound!" << std::endl;
	}


	//  initialize fonts
	std::cerr << "Initializing fonts..." << std::endl;
	const int buttonFontSize = 30;
	wonTextFont = al_load_font("fonts/DejaVuSans.ttf", buttonFontSize, NULL);
	if (wonTextFont == NULL)
	{
		printf("Unable to load font!\n");
	}


	//  initialize logic thread
	logicThread = al_create_thread(LogicThread, (void*)this);
	if (logicThread == NULL)
	{
		std::cerr << "Unable to create allegro thread for logic frames!" << std::endl;
	}
	logicThreadWorking = false;

	//  initialize socket receive thread
	receiveThread = al_create_thread(ReceiveThread, (void*)this);
	if (receiveThread == NULL)
	{
		std::cerr << "Unable to create allegro thread for reading sockets!" << std::endl;
	}
	receiveThreadWorking = false;


	//  read config file
	std::ifstream configFile("config/config.ini");
	if (!configFile.fail())
	{
		//  config file open success
		//read key=value pairs line by line
		//TODO: Create a string split function
		std::string line;
		while (std::getline(configFile, line))
		{
			size_t delimPos = line.find('=');
			std::string key = line.substr(0, delimPos);
			std::string value = line.substr(delimPos+1, std::string::npos);
			std::cerr << "Config " << key << "=" << value << std::endl;
			configMap[key] = value;
		}
	}
	else
	{
		//  unable to open config file
		std::cerr << "MultiPlayerState::Init() --- Unable to open config file" << std::endl;
	}

	
	//  command line arguments override config file
	if (GameEngine::isHostSpecified())
	{
		configMap["networktype"] = std::string("host");
	}
	else if (GameEngine::isClientSpecified())
	{
		configMap["networktype"] = std::string("client");
	}
	

	/*
	playername=oyuncu, string
	hostip=127.0.0.1, a.b.c.d
	hostport=6500, N
	networktype=host, client
	numclients=1
	*/

	hostPort = std::stoi(configMap["hostport"]);
	numClients = std::stoi(configMap["numclients"]);
	playerName = configMap["playername"];


	//  initialize socket system
	bool success = Socket::InitializeSockets();
	if (!success)
	{
		std::cerr << "MultiPlayerState::Init() --- Unable to initialize socket system!" << std::endl;
	}

	if (configMap["networktype"] == "host")
	{
		InitHost();
	}
	else if (configMap["networktype"] == "client")
	{
		InitClient();
	}
	else
	{
		std::cerr << "Invalid networktype in config file!" << std::endl;
	}

	currentLockstepTurn = 0;



	//  start render thread
	//TODO:  maybe this is not the right way?
	


	Draw2();
	for (int i = 0; i < 3; ++i)
	{
		al_play_sample(tickSound, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
		al_rest(1.0);
	}

	//  start logic thread
	logicThreadWorking = true;
	al_start_thread(logicThread);

	//  start socket receiver thread
	receiveThreadWorking = true;
	al_start_thread(receiveThread);


	printf("MultiPlayerState Init\n");
}

void MultiPlayerState::Cleanup()
{
	//SDL_FreeSurface(bg);
	//SDL_FreeSurface(fader);

	printf("MultiPlayerState Cleanup\n");
}

void MultiPlayerState::Pause()
{
	printf("MultiPlayerState Pause\n");
}

void MultiPlayerState::Resume()
{
	printf("MultiPlayerState Resume\n");
}

void MultiPlayerState::HandleEvents(GameEngine* game)
{
	if (game->event.type == ALLEGRO_EVENT_KEY_DOWN || game->event.type == ALLEGRO_EVENT_KEY_UP)
	{
		//  flag is true if event is KEY_DOWN, else false
		bool pressFlag = (game->event.type == ALLEGRO_EVENT_KEY_DOWN) ? true : false;

		//  arrowKeyFlag is true if key is an arrow key
		int keycode = game->event.keyboard.keycode;
		bool arrowKeyFlag = (keycode == ALLEGRO_KEY_LEFT 
						   ||keycode == ALLEGRO_KEY_RIGHT
						   ||keycode == ALLEGRO_KEY_UP
						   ||keycode == ALLEGRO_KEY_DOWN);

		if (arrowKeyFlag)
		{
			//  if event is a arrow key press event, set other directions to false first
			if (pressFlag)
			{
				pressedKeys[ALLEGRO_KEY_LEFT] = false;
				pressedKeys[ALLEGRO_KEY_RIGHT] = false;
				pressedKeys[ALLEGRO_KEY_UP] = false;
				pressedKeys[ALLEGRO_KEY_DOWN] = false;

				std::cerr << "arrow key press event" << std::endl;
			}


			if (arrowKeyFlag && pressFlag)
			{
				pressedKeys[keycode] = pressFlag;
			}

		}
	}
	
}

void MultiPlayerState::Update(GameEngine* game) 
{
	if (gameFinishSignal)
	{
		//  Draw one last time
		Draw(game);


		//  destroy receiver thread
		if (receiveThread != NULL)
    		al_destroy_thread(receiveThread);

		//  destroy logic thread
		if (logicThread != NULL)
    		al_destroy_thread(logicThread);

		//  close sockets

		


		//  end game
		game->SetRunning(false);

	}
}

void MultiPlayerState::Draw(GameEngine* game) 
{
	//std::cerr << "Draw() start" << std::endl;
	//  clear screen
	al_clear_to_color(al_map_rgba(0, 0, 0, 255));

	//  TODO: These variables may need a mutex protection
	//because they might be modified from logic thread!
	size_t positionCount = 0;
	for (size_t i = 0; i < playerCount; ++i)
	{
		positionCount = std::max(positionCount, playerAllX[i].size());
	}
	//size_t positionCount = playerAllX[0].size();
	size_t lastTimeIndex = positionCount-2;  //lockstepTurnFinishTimesMilliseconds.size()-1;
	//std::cerr << "positionCount:" << positionCount << std::endl;


	//  interpolate when drawing last rectangle
	//  calculate expected lockstep turn time
	uint64_t expectedDelayMilliseconds;
	
	if (currentLockstepTurn == 0 || currentLockstepTurn == 1)
	{
		//  TODO: this should be depend on logicFPS variable or some variable, not constant
		expectedDelayMilliseconds = 100;
	}
	else
	{
		//  TODO: Use one (or a few) more frames instead of difference between last two frames
		expectedDelayMilliseconds = lockstepTurnFinishTimesMilliseconds[lastTimeIndex] - lockstepTurnFinishTimesMilliseconds[lastTimeIndex-1];
	}

	//  get current time in milliseconds
	uint64_t currentTimeMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::system_clock::now().time_since_epoch()
			).count();

	//  diff: amount of milliseconds between current time and end of last lockstep turn
	uint64_t diff;
	if (currentLockstepTurn == 0 || currentLockstepTurn == 1)
	{
		diff = expectedDelayMilliseconds;
	}
	else
	{
		diff = currentTimeMilliseconds - lockstepTurnFinishTimesMilliseconds[lastTimeIndex];
	}
	
	float interpolateRatio = (float)diff / (float)expectedDelayMilliseconds;
	//std::cerr << "diff:" << diff << std::endl;
	//std::cerr << "interpolation ratio:" << interpolateRatio << std::endl;
	//std::cerr << "Draw() after time calculations" << std::endl;

	if (playerAllX[0].size() <= 2)
	{
		return;
	}


	//  TODO: maybe prune out of screen areas when drawing rectangles

	
	for (int i = 0; i < playerCount; ++i)
	{
		//  draw rectangles between two positions, except last pair
		size_t playerPositionCount;
		if (playerAlive[i])
			playerPositionCount = positionCount;
		else
			playerPositionCount = playerAllX[i].size() + 1;  //  +1 so it draws all rectangles including last one
		
		for (size_t j = 0; j < playerPositionCount-2; ++j)
		{
			
			int cx1 = playerAllX[i][j];
			int cy1 = playerAllY[i][j];

			int cx2 = playerAllX[i][j+1];
			int cy2 = playerAllY[i][j+1];


			int x1 = std::min(cx1, cx2) - SNAKE_WIDTH;
			int y1 = std::min(cy1, cy2) - SNAKE_HEIGHT;

			int x2 = std::max(cx1, cx2) + SNAKE_WIDTH;
			int y2 = std::max(cy1, cy2) + SNAKE_HEIGHT;

			al_draw_filled_rectangle(x1, y1, x2, y2, playerColors[i]);
		}
		//if (i==0)
    	//	std::cerr << "last fullblock x2:" << std::max(playerAllX[i][playerPositionCount-3], playerAllX[i][playerPositionCount-2]) + SNAKE_WIDTH << std::endl;
	}
		
	if (true){
	//  draw last rectangle
	if (false || diff >= expectedDelayMilliseconds)
	{
		for (int i = 0; i < playerCount; ++i)
		{
			if (!playerAlive[i])
				continue;
			//  just draw last rectangle as a whole

			int cx1 = playerAllX[i][positionCount-2];
			int cy1 = playerAllY[i][positionCount-2];

			int cx2 = playerAllX[i][positionCount-1];
			int cy2 = playerAllY[i][positionCount-1];
			
			int x1 = std::min(cx1, cx2) - SNAKE_WIDTH;
			int y1 = std::min(cy1, cy2) - SNAKE_HEIGHT;

			int x2 = std::max(cx1, cx2) + SNAKE_WIDTH;
			int y2 = std::max(cy1, cy2) + SNAKE_HEIGHT;

			//if (i==0)
    		//	std::cerr << "largediff x2:" << x2 << std::endl;

			al_draw_filled_rectangle(x1, y1, x2, y2, playerColors[i]);
		}
	}
	else
	{
		//  interpolated draw
		std::cerr << "interpolation ratio:" << interpolateRatio << std::endl;
		for (int i = 0; i < playerCount; ++i)
		{
			if (!playerAlive[i])
				continue;

			int cx1 = playerAllX[i][positionCount-2];
			int cy1 = playerAllY[i][positionCount-2];

			int cx2 = playerAllX[i][positionCount-1];
			int cy2 = playerAllY[i][positionCount-1];


			//  left point
			int x1 = std::min(cx1, cx2) - SNAKE_WIDTH;
			
			//  upper point
			int y1 = std::min(cy1, cy2) - SNAKE_HEIGHT;
			
			//  right point
			int x2 = std::max(cx1, cx2) + SNAKE_WIDTH;
			
			//  down point
			int y2 = std::max(cy1, cy2) + SNAKE_HEIGHT;


			//  get the direction which caused last movement
			uint8_t lastMovementDir = playerAllDir[i][positionCount-1];
			if (lastMovementDir == DIRECTION_LEFT || lastMovementDir == DIRECTION_RIGHT)
			{
				int xDiff = x2-x1;

				if (lastMovementDir == DIRECTION_RIGHT)
				{
    				x2 = x1 + (interpolateRatio*xDiff);
					//std::cerr << "interpolate x2:" << x2 << std::endl;
				}
				else
					x1 = x2 - (interpolateRatio*xDiff);
			}
			else if (lastMovementDir == DIRECTION_UP || lastMovementDir == DIRECTION_DOWN)
			{
				int yDiff = y2-y1;

				if (lastMovementDir == DIRECTION_DOWN)
    				y2 = y1 + (interpolateRatio*yDiff);
				else
					y1 = y2 - (interpolateRatio*yDiff);
			}

			
			

			
			
			
			//int x2 = (int) (interpolateRatio * (std::max(cx1, cx2) + SNAKE_WIDTH));
			//int y2 = (int) (interpolateRatio * (std::max(cy1, cy2) + SNAKE_HEIGHT));

			al_draw_filled_rectangle(x1, y1, x2, y2, playerColors[i]);
		}
	}
	}


	//  if a player won the game, write its name
	if (playerWon)
	{
		char wonTextBuffer[200];
		sprintf(wonTextBuffer, "%s Won!", wonPlayerName.c_str());
		al_draw_text(wonTextFont, playerColors[wonPlayerIndex], int(SCREEN_WIDTH/2), 100, ALLEGRO_ALIGN_CENTER, wonTextBuffer );
	}

		

	if (false && isHost)
	{
		char buffer[5];
		sprintf(buffer, "%04d", renderFrameCounter);
		//std::string frameString = std::to_string(renderFrameCounter);
		std::string frameString(buffer);
		std::string fname = "frame_" + frameString + ".png";
		al_save_bitmap(fname.c_str(), al_get_backbuffer(game->display));
	}
	//std::cerr << "Render frame " << renderFrameCounter << std::endl;
	++renderFrameCounter;
	al_flip_display();
}

void MultiPlayerState::Draw2()
{
	//  clear screen
	al_clear_to_color(al_map_rgba(0, 0, 0, 255));


	for (int i = 0; i < playerCount; ++i)
	{
		int cx = playerX[i];
		int cy = playerY[i];

		int x1 = cx - SNAKE_SIZE;
		int y1 = cy - SNAKE_SIZE;

		int x2 = cx + SNAKE_SIZE;
		int y2 = cy + SNAKE_SIZE;
		al_draw_filled_rectangle(x1, y1, x2, y2, playerColors[i]);
	}

	al_flip_display();
}





void* MultiPlayerState::LogicThread(ALLEGRO_THREAD* thread, void* arg)
{
	MultiPlayerState* cptr = (MultiPlayerState*) arg;


	std::cerr << "Logic thread, number of players:" << cptr->playerCount << std::endl;


	//  create an event queue for display thread
	ALLEGRO_EVENT_QUEUE* logicEventQueue = al_create_event_queue();
    if (!logicEventQueue)
    {
        std::cerr << "Unable to create allegro event queue for logic thread!" << std::endl;
    }

	//  create a timer for logic thread
	int logicFPS = 10;
	ALLEGRO_TIMER* logicTimer = al_create_timer(1.0 / logicFPS);
	if (logicTimer == NULL)
	{
		std::cerr << "Unable to create allegro timer for logic thread!" << std::endl;
	}
	

	//  register logicTimer events to logicEventQueue
	al_register_event_source(logicEventQueue, al_get_timer_event_source(logicTimer));
	


	//  RIGHT, UP, LEFT, DOWN
	//  0, 1, 2, 3
	static int dx[] = {1, 0, -1, 0};
	static int dy[] = {0, -1, 0, 1};

	al_start_timer(logicTimer);
	cptr->logicFrameCounter = 0;
	while (cptr->logicThreadWorking)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(logicEventQueue, &ev);
		if (ev.type == ALLEGRO_EVENT_TIMER)
		{
			if (ev.timer.source == logicTimer)
			{
				//  logic stuff here
				//std::cerr << "deleteme logic begin" << std::endl;

				//  STEP 1- Check if something need to be changed
				bool canExecuteTurn = false;
				if (cptr->currentLockstepTurn == 0
					|| cptr->currentLockstepTurn == 1)
				{
					//  nothing needs to be changed
					canExecuteTurn = true;
				}
				else
				{
					if (cptr->turnMessages.size() <= cptr->currentLockstepTurn-2)//if (cptr->turnMessages[cptr->currentLockstepTurn-2].size() <= 0)
					{
						canExecuteTurn = false;
					}
					else
					{
						//  perform necessary changes
						for (size_t i = 0; i < cptr->turnMessages[cptr->currentLockstepTurn-2].size(); ++i)
						{
							Packet* message = cptr->turnMessages[cptr->currentLockstepTurn-2][i];
							if (message->GetPacketType() == PACKET_TYPE_DO_NOTHING)
							{
								//  do nothing
							}
							else if (message->GetPacketType() == PACKET_TYPE_DIRECTION)
							{
								//  change player direction accordingly
								int playerId = (static_cast<PacketDirection*>(message))->playerId;
								cptr->playerDir[playerId] = (static_cast<PacketDirection*>(message))->direction;
							}
						}

						canExecuteTurn = true;
					}
				}

				
				bool playerDeadThisTurn = false;
				if (canExecuteTurn)
				{
					//  STEP 2- Apply actions that needs to be performed this turn
					for (int i = 0; i < cptr->playerCount; ++i)
					{
						if (cptr->playerAlive[i])
						{
							//  at each timestep save dir
							cptr->playerAllDir[i].push_back(cptr->playerDir[i]);

							int newX = cptr->playerX[i] + dx[cptr->playerDir[i]] * SNAKE_SPEED;
							int newY = cptr->playerY[i] + dy[cptr->playerDir[i]] * SNAKE_SPEED;

							//  TODO: Collision check here

							//  case 1, player collides with already existing area
							//check traces of all players
							for (int j = 0; j < cptr->playerCount; ++j)
							{
								for (size_t k = 0; k < cptr->playerAllX[j].size()-1; ++k)
								{
									int cx1 = cptr->playerAllX[j][k];
									int cy1 = cptr->playerAllY[j][k];

									int cx2 = cptr->playerAllX[j][k+1];
									int cy2 = cptr->playerAllY[j][k+1];

									int x1 = std::min(cx1, cx2) - SNAKE_WIDTH;
									int y1 = std::min(cy1, cy2) - SNAKE_HEIGHT;

									int x2 = std::max(cx1, cx2) + SNAKE_WIDTH;
									int y2 = std::max(cy1, cy2) + SNAKE_HEIGHT;

									//  check if rectangle (x1, y1, x2, y2) contains newX newY
									if (newX >= x1 && newX <= x2
										&&  newY >= y1 && newY <= y2)
									{
										al_play_sample(cptr->crashSound, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, NULL);
										std::cerr << cptr->playerNames[j] << " killed " << cptr->playerNames[i] << std::endl;
										cptr->playerAlive[i] = false;

										//  a player died this turn
										playerDeadThisTurn = true;

										

										goto END_COLLISION;
									}
								}
							}
							END_COLLISION:

							cptr->playerX[i] = newX;
							cptr->playerY[i] = newY;

							cptr->playerAllX[i].push_back(cptr->playerX[i]);
							cptr->playerAllY[i].push_back(cptr->playerY[i]);
						}
					}


					


					//  STEP 3- Send new messages for future turns
					LogicThreadLockstepBroadcast(cptr);

					uint64_t currentTimeMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
						std::chrono::system_clock::now().time_since_epoch()
						).count();
					cptr->lockstepTurnFinishTimesMilliseconds.push_back(currentTimeMilliseconds);
				
					++(cptr->currentLockstepTurn);
				}
				else
				{
					std::cerr << "skipped lockstep turn" << std::endl;
				}
				
				std::cerr << "Logic frame " << cptr->logicFrameCounter << ", LockstepTurn:" << cptr->currentLockstepTurn << std::endl;
				
				
				
				++cptr->logicFrameCounter;

				//  check if game is finished
				if (playerDeadThisTurn)
				{	
					//  check if all players are dead
					//  also count number of alive players
					bool allDead = true;
					int alivePlayerCount = 0;
					int alivePlayerIndex = -1;
					for (size_t cihan = 0; cihan < cptr->playerCount; ++cihan)
					{
						if (cptr->playerAlive[cihan])
						{
							std::cerr << "player " << cihan << " is still alive" << std::endl;
							++alivePlayerCount;
							alivePlayerIndex = cihan;
							allDead = false;
						}
					}

					if (alivePlayerCount == 1)
					{
						cptr->playerWon = true;
						cptr->wonPlayerName = cptr->playerNames[alivePlayerIndex];
						cptr->wonPlayerIndex = alivePlayerIndex;
					}

					if (allDead)
					{
						//  all players are dead, finish the game
						cptr->gameFinishSignal = true;

						//  signal receive thread to finish
						cptr->receiveThreadWorking = false;


						//  finish this logic thread
						//after some time
						al_rest(2.0);
						return NULL;

						

					}
				}
			}
		}
	}

	std::cerr << "End of logic thread" << std::endl;
}

bool MultiPlayerState::LogicThreadLockstepBroadcast(MultiPlayerState* cptr)
{
	//  first, detect current action of player 
	//regardless of its host or client
	uint8_t decidedDirection;

	if (cptr->pressedKeys[ALLEGRO_KEY_RIGHT])
	{
		decidedDirection = DIRECTION_RIGHT;
	}
	else if (cptr->pressedKeys[ALLEGRO_KEY_UP])
	{
		decidedDirection = DIRECTION_UP;
	}
	else if (cptr->pressedKeys[ALLEGRO_KEY_LEFT])
	{
		decidedDirection = DIRECTION_LEFT;
	}
	else if (cptr->pressedKeys[ALLEGRO_KEY_DOWN])
	{
		decidedDirection = DIRECTION_DOWN;
	}
	else
	{
		//  do not change the direction
		//will cause creation of a different packet class (at least for now)
		decidedDirection = DIRECTION_SAME;
	}



	if (cptr->isHost)
	{
		//  host only stores its messages until messages from all clients are received
		if (decidedDirection == DIRECTION_SAME)
		{
			PacketDoNothing* packet = new PacketDoNothing(cptr->playerId, cptr->currentLockstepTurn);
			cptr->selfMessages.push_back(packet);
		}
		else
		{
			PacketDirection* packet = new PacketDirection(cptr->playerId, decidedDirection, cptr->currentLockstepTurn);
			cptr->selfMessages.push_back(packet);
		}

		std::cerr << "new selfMessage:" << (int) (cptr->selfMessages[cptr->selfMessages.size()-1]->GetPacketType()) << ", size:" << cptr->selfMessages.size() << std::endl;

		HostBroadcastMessages(cptr->selfMessages.size()-1, cptr);

		//  sending of all messages done in receive thread
	}
	else
	{
		//  client sends its action to host
		if (decidedDirection == DIRECTION_SAME)
		{
			PacketDoNothing packet(cptr->playerId, cptr->currentLockstepTurn);
			cptr->clientSocket.Send(packet);
		}
		else
		{
			PacketDirection packet(cptr->playerId, decidedDirection, cptr->currentLockstepTurn);
			cptr->clientSocket.Send(packet);
		}

	}

	return true;
}


void* MultiPlayerState::ReceiveThread(ALLEGRO_THREAD* thread, void* arg)
{
	MultiPlayerState* cptr = (MultiPlayerState*) arg;


	//  While inside if statement for more efficiency???
	while (cptr->receiveThreadWorking)
	{
		
		//  expand turnMessages vector if necessary
		if (cptr->turnMessages.size() < cptr->currentLockstepTurn+1)
		{
			cptr->turnMessages.push_back(std::vector<Packet*>());
		}


		if (cptr->isHost)
		{
			//  host receive thread
			//  TODO: Maybe also listen for accepts here?
			//  host listens commands from clients

			//  if new lockstep turn add a new vector for messages of that turn
			if (cptr->clientMessages.size() < cptr->currentLockstepTurn + 1)
			{
				
		        cptr->clientMessages.push_back(std::vector<Packet*>());
			}

			//  do nothing in first turn
			if (cptr->currentLockstepTurn == 0)
			{
				continue;
			}


			//  after first turn, start receiving messages from all clients
			const int timeout = 0;  //  milliseconds
			for (int i = 0; i < cptr->numClients; ++i)
			{
				bool readyToRead = (cptr->acceptSockets[i].ReceivePoll(timeout) > 0);
				if (readyToRead)
				{
					//std::cerr << "acceptSocket[" << i << "] is ready to receive." << std::endl;
					Packet* receivedPacket = cptr->acceptSockets[i].Receive();
					if (receivedPacket == NULL)
					{
						//  better error handling/printing
					}
					else
					{
						uint8_t packetType = receivedPacket->GetPacketType();
						uint16_t packetSize = receivedPacket->GetBufferSize();
						fprintf(stderr, "Packet arrived, protocolId:%d, seqId:%d\n", receivedPacket->GetProtocolNumber(), receivedPacket->GetSequenceId());
						
						
						uint32_t packetTurn;

						//  TODO: Check if packetTurn belongs to a future turn
						//because a client with false packet may crash host's game
						
						if (packetType == PACKET_TYPE_DIRECTION)
						{
							fprintf(stderr, "Direction package arrived, size(excluding header):%d\n", packetSize);
							PacketDirection* directionPackage = (static_cast<PacketDirection*> (receivedPacket));
							//PacketDirection directionPackage = *(static_cast<PacketDirection*> (receivedPacket));
							fprintf(stderr, "direction value:%d\n", directionPackage->direction);
							packetTurn = directionPackage->lockstepTurn;
							cptr->clientMessages[packetTurn].push_back(directionPackage);
							//cptr->clientMessages[packetTurn][i] = &directionPackage;
						}
						else if (packetType == PACKET_TYPE_DO_NOTHING)
						{
							fprintf(stderr, "Do nothing package arrived, size(excluding header):%d\n", packetSize);
							PacketDoNothing* nothingPackage = (static_cast<PacketDoNothing*> (receivedPacket));
							//PacketDoNothing nothingPackage = *(static_cast<PacketDoNothing*> (receivedPacket));
							packetTurn = nothingPackage->lockstepTurn;
							cptr->clientMessages[packetTurn].push_back(nothingPackage);
							//cptr->clientMessages[packetTurn][i] = &nothingPackage;
						}

						//  TODO: Maybe align playerId and message (second) index array

						//  this check is only here for debug purposes
						if (cptr->clientMessages[packetTurn].size() == cptr->numClients
							&&  cptr->lastSentTurn == packetTurn-1
							/*&&  cptr->selfMessages.size() > packetTurn*/ /*why is this necessary*/)
						{
							std::cerr << "Host --- Received all " << cptr->numClients << " messages for turn " << packetTurn << std::endl;
							
							bool broadcastOk = cptr->HostBroadcastMessages(packetTurn, cptr);
							
						}
					}
				}
			}
			
			al_rest(0.0010);  //  10 ms
		}
		else
		{
			//  client listens messages from host
			const int timeout = 10;  //  milliseconds
			bool readyToRead = (cptr->clientSocket.ReceivePoll(timeout) > 0);
			if (readyToRead)
			{
				Packet* receivedPacket = cptr->clientSocket.Receive();
				if (receivedPacket == NULL)
				{
					//  better error handling/printing
				}
				else
				{
					uint8_t packetType = receivedPacket->GetPacketType();
					uint16_t packetSize = receivedPacket->GetBufferSize();
					fprintf(stderr, "Packet arrived, protocolId:%d, seqId:%d\n", receivedPacket->GetProtocolNumber(), receivedPacket->GetSequenceId());
					if (packetType == PACKET_TYPE_ACTIONS)
					{
						fprintf(stderr, "Actions package arrived, size(excluding header):%d\n", packetSize);
						PacketActions* actionsPackage = (static_cast<PacketActions*> (receivedPacket));
						//PacketActions actionsPackage = *(static_cast<PacketActions*> (receivedPacket));
						fprintf(stderr, "action count:%d, lockstep turn:%d\n", actionsPackage->actionCount, actionsPackage->lockstepTurn);

						//  save received turn messages to be used in logic thread
						std::cerr << "Client --- Received turn messages for turn " << actionsPackage->lockstepTurn << std::endl;
						cptr->turnMessages[actionsPackage->lockstepTurn] = *(actionsPackage->actions);

						//  remember last received turn
						cptr->lastReceivedTurn = actionsPackage->lockstepTurn;
					}
					else
					{
						std::cerr << "Client --- Received a message whose type is not ACTIONS?" << std::endl;
					}
				}
			}
		}
	}
}



/*
	This function is called from two places
	1- When host player receive all messages for a turn X
	2- When host player decides its own message for turn X
*/
bool MultiPlayerState::HostBroadcastMessages(uint32_t packetTurn, void* arg)
{
	MultiPlayerState* cptr = (MultiPlayerState*) arg;

	/*
	if (cptr->clientMessages[packetTurn].size() == cptr->numClients
							&&  cptr->lastSentTurn == packetTurn-1
							&&  cptr->selfMessages.size() > packetTurn)
	*/

	if (cptr->clientMessages[packetTurn].size() == cptr->numClients
								&&  cptr->lastSentTurn == packetTurn-1
								&&  cptr->selfMessages.size() > packetTurn)
	{
		//  group all messages of the turn from clients together
		std::vector<Packet*>* messagesToSent = new std::vector<Packet*>();
		for (size_t j = 0; j < cptr->numClients; ++j)
		{
			messagesToSent->push_back(cptr->clientMessages[packetTurn][j]);
		}
		//std::cerr << "deleteme1" << std::endl;
		//  also include the host's own message
		messagesToSent->push_back(cptr->selfMessages[packetTurn]);
		//std::cerr << "deleteme2" << std::endl;

		std::cerr << "selfMessages size:" << cptr->selfMessages.size() << std::endl;
		std::cerr << "packetTurn:" << packetTurn << std::endl;
		std::cerr << (int) (cptr->selfMessages[packetTurn]->GetPacketType()) << std::endl;

		PacketActions actionsPacket(messagesToSent, packetTurn);
		//std::cerr << "deleteme3" << std::endl;

		//  send it to all clients
		std::cerr << "Host --- Sending messages of turn " << packetTurn << " to all clients." << std::endl;
		for (int j = 0; j < cptr->numClients; ++j)
		{
			cptr->acceptSockets[j].Send(actionsPacket);
		}
		//std::cerr << "deleteme4" << std::endl;

		

		//  keep a copy for yourself
		//std::cerr << "deleteme5" << std::endl;
		for (size_t j = 0; j < messagesToSent->size(); ++j)
		{
			cptr->turnMessages[packetTurn].push_back(  (*messagesToSent)[j] );
		}
		//std::cerr << "deleteme6" << std::endl;
		cptr->turnMessages[packetTurn].push_back(cptr->selfMessages[packetTurn]);
		//std::cerr << "deleteme7" << std::endl;

		//  remember last sent turn
		cptr->lastSentTurn = packetTurn;
	}

}