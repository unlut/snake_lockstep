#ifndef INTROSTATE_H
#define INTROSTATE_H

#include <allegro5/allegro.h>
#include "allegro5/allegro_image.h"
#include "gamestate.h"

class IntroState : public GameState
{
public:
	void Init();
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents(GameEngine* game);
	void Update(GameEngine* game);
	void Draw(GameEngine* game);

	static IntroState* Instance() 
    {
		return &m_IntroState;
	}

protected:
	IntroState() { }

private:
	static IntroState m_IntroState;

	//SDL_Surface* bg;
	//SDL_Surface* fader;
	//int alpha;
	ALLEGRO_BITMAP  *introImage   = NULL;
	

	const float INTRO_DURATION = 1.0;  //  seconds
	const int INTRO_START_ALPHA = 0;  // [0, 255]
	const int INTRO_END_ALPHA = 255;

	int introImageAlpha = INTRO_START_ALPHA;  // [0, 255]
};

#endif
