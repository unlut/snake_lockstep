
#include <stdio.h>

#include <allegro5/allegro.h>


#include "gameengine.h"
#include "gamestate.h"
#include "introstate.h"
#include "playstate.h"
#include "macros.h"
#include "menustate.h"
#include "menubutton.h"
#include "multiplayerstate.h"

MenuState MenuState::m_MenuState;

void MenuState::Init()
{
	/*

	MenuButton singlePlayerButton("Single Player", x, y, width, height, font, 
	defaultEdgeColor, defaultInnerColor,
	hoverEdgeColor, hoverInnerColor); // clickedFunc???
	
	*/
	drawFPS = GAME_MENU_FPS;
	mainThreadFPS = drawFPS;

	const int buttonFontSize = 30;
	menuButtonFont = al_load_font("fonts/DejaVuSans.ttf", buttonFontSize, NULL);

	if (menuButtonFont == NULL)
	{
		printf("Unable to load font!\n");
	}


	menuButtonDefaultInnerColor = al_map_rgba(0, 127, 127, 255);
	menuButtonDefaultEdgeColor = al_map_rgba(255, 255, 255, 255);
	menuButtonHoverInnerColor = al_map_rgba(255, 255, 255, 255);
	menuButtonHoverEdgeColor = al_map_rgba(255, 255, 255, 255);

	int x, y, width, height;
	
	y = 50;
	width = 300;
	height = 100;
	x = (SCREEN_WIDTH/2) - (width/2);

	singlePlayerButton = new MenuButton("Single Player", x, y, width, height, menuButtonFont, 
	COLOR_WHITE, COLOR_BLACK,
	menuButtonDefaultEdgeColor, menuButtonDefaultInnerColor,
	menuButtonHoverEdgeColor, menuButtonHoverInnerColor);

	y = 200;
	x = x;
	width = width;
	height = height;
	multiPlayerButton = new MenuButton("Multi Player", x, y, width, height, menuButtonFont, 
	COLOR_WHITE, COLOR_BLACK,
	menuButtonDefaultEdgeColor, menuButtonDefaultInnerColor,
	menuButtonHoverEdgeColor, menuButtonHoverInnerColor);


	//  get current mouse position
	if (!al_is_mouse_installed())
	{
		printf("Mouse not installed!\n");
	}
	ALLEGRO_MOUSE_STATE mouseState;
	al_get_mouse_state(&mouseState);
	currentMouseX = mouseState.x;
	currentMouseY = mouseState.y;

	printf("Current mouse pos:%d %d\n", currentMouseX, currentMouseY);

	printf("MenuState Init\n");
}

void MenuState::Cleanup()
{
	//SDL_FreeSurface(bg);
	//SDL_FreeSurface(fader);

	printf("MenuState Cleanup\n");
}

void MenuState::Pause()
{
	printf("MenuState Pause\n");
}

void MenuState::Resume()
{
	printf("MenuState Resume\n");
}

void MenuState::HandleEvents(GameEngine* game)
{
	if (game->event.type == ALLEGRO_EVENT_MOUSE_AXES)
	{
		currentMouseX = game->event.mouse.x;
		currentMouseY = game->event.mouse.y;
	}
	else if (game->event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
	{
		game->SetRunning(false);
	}
	else if (game->event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
	{
		int clickedX = game->event.mouse.x;
		int clickedY = game->event.mouse.y;

		bool singleButtonClicked = singlePlayerButton->IsPointInsideButton(clickedX, clickedY);
		if (singleButtonClicked)
		{
			printf("Go to single player\n");
			game->SetRunning(false);
		}

		bool multiButtonClicked = multiPlayerButton->IsPointInsideButton(clickedX, clickedY);
		if (multiButtonClicked)
		{
			printf("Go to multi player\n");

			game->ChangeState(MultiPlayerState::Instance());
			//game->SetRunning(false);
		}

	}
	
}

void MenuState::Update(GameEngine* game) 
{
	
}

void MenuState::Draw(GameEngine* game) 
{
	if (game->event.type == ALLEGRO_EVENT_TIMER)
	{
		if (game->event.timer.source == game->mainTimer)
		{
			//  clear screen
			al_clear_to_color(al_map_rgba(0, 0, 0, 255));

			//  draw buttons
			bool hovered = singlePlayerButton->IsPointInsideButton(currentMouseX, currentMouseY);
			singlePlayerButton->Draw(hovered);

			hovered = multiPlayerButton->IsPointInsideButton(currentMouseX, currentMouseY);
			multiPlayerButton->Draw(hovered);
			
			
			al_flip_display();
		}
	}
}
