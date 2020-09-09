#ifndef GAMEENGINE_H
#define GAMEENGINE_H


//  include allegro somehow
#include <allegro5/allegro.h>
#include "allegro5/allegro_image.h"
#include <vector>





//  forward declaration of GameState
class GameState;


class GameEngine
{
private:
    //  stack of states
    std::vector<GameState*> states;
    
    bool running;
    bool fullScreen;

    bool singlePlayer;
    bool multiPlayer;


    //  if user specify being a client or host via command line arguments
    bool hostClientSpecified;
    static bool hostSpecified;
    static bool clientSpecified;

public:
    
    void Init(const char* title, int width=800, int height=600,
              int bpp=0, bool fullScreen=false);
    void Cleanup();
    
    void ChangeState(GameState* state);
    void PushState(GameState* state);
    void PopState();
    
    void HandleEvents();
    void Update();
    void Draw();
    
    bool Running() { return running; };
    void SetRunning(bool running) {this->running = running;}
    void Quit() { running=false; };

    bool isSinglePlayer() { return singlePlayer; }
    void SetSinglePlayer() { singlePlayer = true; }
    void SetMultiplayer() { multiPlayer = true; }

    static bool isHostSpecified() { return hostSpecified; }
    static bool isClientSpecified() {return clientSpecified; }
    static void SetHostSpecified(bool hostSpecified) { GameEngine::hostSpecified = hostSpecified; }
    static void SetClientSpecified(bool clientSpecified) { GameEngine::clientSpecified = clientSpecified; }

    GameState* GetCurrentState();
    
    //  ALLEGRO SCREEN VVARIABLE BURAYA??
    ALLEGRO_DISPLAY* display = NULL;
    ALLEGRO_EVENT_QUEUE* eventQueue = NULL;
    ALLEGRO_EVENT_QUEUE* drawEventQueue = NULL;
    
    ALLEGRO_EVENT event;
    ALLEGRO_EVENT drawEvent;

    ALLEGRO_TIMER* mainTimer;
    ALLEGRO_TIMER* drawTimer;
    ALLEGRO_TIMER* logicTimer;
};


#endif
