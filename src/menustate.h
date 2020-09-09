#ifndef MENUSTATE_H
#define MENUSTATE_H

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


class MenuState : public GameState
{
public:
	void Init();
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents(GameEngine* game);
	void Update(GameEngine* game);
	void Draw(GameEngine* game);

	static MenuState* Instance() 
    {
		return &m_MenuState;
	}

protected:
	MenuState() { }

private:
	static MenuState m_MenuState;

	//SDL_Surface* bg;
	//SDL_Surface* fader;
	//int alpha;
	

	ALLEGRO_FONT* menuButtonFont;
	ALLEGRO_COLOR menuButtonDefaultInnerColor;
	ALLEGRO_COLOR menuButtonDefaultEdgeColor;
	ALLEGRO_COLOR menuButtonHoverInnerColor;
	ALLEGRO_COLOR menuButtonHoverEdgeColor;


	//  maybe a list of active buttons?

    MenuButton* singlePlayerButton;
	MenuButton* multiPlayerButton;


	int currentMouseX;
	int currentMouseY;
};

#endif
