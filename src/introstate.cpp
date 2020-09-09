
#include <stdio.h>

#include <allegro5/allegro.h>
#include "gameengine.h"
#include "gamestate.h"
#include "introstate.h"
#include "playstate.h"
#include "menustate.h"
#include "macros.h"

IntroState IntroState::m_IntroState;

void IntroState::Init()
{
	
	introImage = al_load_bitmap("images/himhim1.bmp");
	if (introImage == NULL)
	{
		printf("IntroState - introImage NULL\n");
	}

	introImageAlpha = INTRO_START_ALPHA;

	drawFPS = GAME_INTRO_FPS;
	mainThreadFPS = drawFPS;


	/*
	SDL_Surface* temp = SDL_LoadBMP("intro.bmp");

	bg = SDL_DisplayFormat(temp);

	SDL_FreeSurface(temp);

	// create the fader surface like the background with alpha
	fader = SDL_CreateRGBSurface( SDL_SRCALPHA, bg->w, bg->h, 
								  bg->format->BitsPerPixel, 
								  bg->format->Rmask, bg->format->Gmask, 
								  bg->format->Bmask, bg->format->Amask );

	// fill the fader surface with black
	SDL_FillRect (fader, NULL, SDL_MapRGB (bg->format, 0, 0, 0)) ;

	// start off opaque
	alpha = 255;

	SDL_SetAlpha(fader, SDL_SRCALPHA, alpha);
	*/

	printf("IntroState Init\n");
}

void IntroState::Cleanup()
{
	//SDL_FreeSurface(bg);
	//SDL_FreeSurface(fader);

	printf("IntroState Cleanup\n");
}

void IntroState::Pause()
{
	printf("IntroState Pause\n");
}

void IntroState::Resume()
{
	printf("IntroState Resume\n");
}

void IntroState::HandleEvents(GameEngine* game)
{
	/*
	SDL_Event event;

	if (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				game->Quit();
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_SPACE:
						game->ChangeState( CPlayState::Instance() );
						break;

					case SDLK_ESCAPE:
						game->Quit();
						break;
				}
				break;
		}
	}
	*/
}

void IntroState::Update(GameEngine* game) 
{
	/*
	alpha--;

	if (alpha < 0)
		alpha = 0;

	SDL_SetAlpha(fader, SDL_SRCALPHA, alpha);
	*/


	if (game->event.type == ALLEGRO_EVENT_TIMER)
	{
		if (game->event.timer.source == game->mainTimer)
		{
			float introDeltaAlpha = ((float)(INTRO_END_ALPHA - INTRO_START_ALPHA) / INTRO_DURATION) / (float) GAME_MENU_FPS; 
			introImageAlpha += (int)introDeltaAlpha;
			if (introImageAlpha > 255) 
			{
				introImageAlpha=255;
				printf("End of intro\n");
				game->ChangeState(MenuState::Instance());
			}
			printf("%d\n", introImageAlpha);

			/*
			float intro_fps = GAME_INTRO_FPS;
			float intro_sleep_dt = 1.0 / intro_fps;
			al_rest(intro_sleep_dt);
			*/
		}
		
	}
}

void IntroState::Draw(GameEngine* game) 
{
	/*
	SDL_BlitSurface(bg, NULL, game->screen, NULL);

	// no need to blit if it's transparent
	if ( alpha != 0 )
		SDL_BlitSurface(fader, NULL, game->screen, NULL);

	SDL_UpdateRect(game->screen, 0, 0, 0, 0);
	*/

    
	if (game->event.type == ALLEGRO_EVENT_TIMER)
	{
		if (game->event.timer.source == game->mainTimer)
		{
			//al_draw_bitmap(introImage,200,200,0);
			//al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
			al_clear_to_color(al_map_rgba(0, 0, 0, 255));
			//al_draw_tinted_bitmap(introImage, al_map_rgba(255, 255, 255, 200), 200, 200, 0);
			al_draw_tinted_bitmap(introImage, al_map_rgba(introImageAlpha, introImageAlpha, introImageAlpha, introImageAlpha), 200, 200, 0);
			

			al_flip_display();
			//al_rest(0.2);
		}
		
	}
    

}
