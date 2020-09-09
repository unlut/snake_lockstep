#include <iostream>

#include "gameengine.h"
#include "introstate.h"
#include "macros.h"

int main (int argc, char *argv[])
{
	GameEngine game;

	// initialize the engine
	game.Init( "Engine Test v1.0", SCREEN_WIDTH, SCREEN_HEIGHT);


	//  todo:  maybe move these to game engine Init
	if (argc > 1)
	{
		std::string arg1(argv[1]);
		std::cerr << "argv[1]:" << arg1 << std::endl;
		if (arg1 == "host")
		{
			GameEngine::SetHostSpecified(true);
		}
		else if (arg1 == "client")
		{
			GameEngine::SetClientSpecified(true);
			std::cerr << "client specified" << std::endl;
		}
	}


	//  create render timer
	game.mainTimer = al_create_timer(1.0 / GAME_INTRO_FPS);
	al_register_event_source(game.eventQueue, al_get_timer_event_source(game.mainTimer));
	//al_start_timer(game.mainTimer);


	// load the intro
	game.ChangeState( IntroState::Instance() );


	//  flush if somehow an event got into the queue
	//  TODO: maybe flush event queue when changing states?
	al_flush_event_queue(game.eventQueue);

	// main loop
	while ( game.Running() )
	{
		al_wait_for_event(game.eventQueue, &game.event);

		//  event is implicity handled in below functions?

		game.HandleEvents();
		game.Update();
		game.Draw();
	}

	// cleanup the engine
	game.Cleanup();

	return 0;
}
