#include <iostream>

using namespace std;

//  INCLUDE ALLEGRO
#include <allegro5/allegro.h>
#include "allegro5/allegro_image.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>



#include "gameengine.h"
#include "gamestate.h"

#include "macros.h"

/*
#include <GL/gl.h> 
#include <GL/glu.h> 
#include <GL/glx.h>
*/


bool GameEngine::hostSpecified = false;
bool GameEngine::clientSpecified = false;

void GameEngine::Init(const char* title, int width, int height,
                      int bpp, bool fullScreen)
{
    //  TODO:  Organize these command line argument and their init stuff
    this->hostSpecified = false;
    this->clientSpecified = false;


    //  initialize all allegro stuff here
    if (!al_init())
    {
        cerr << "Unable to initialize allegro!" << endl;
        return;
    }
    if(!al_install_keyboard()) 
    {
        cerr << "Unable to initialize allegro keyboard!" << endl;
        return;
    }
    if (!al_install_mouse())
    {
        cerr << "Unable to initialize allegro mouse!" << endl;
        return;
    }
    if(!al_init_image_addon()) 
    {
        cerr << "Unable to initialize allegro image addon!" << endl;
        return;
    }
    if (!al_init_primitives_addon())
    {
        cerr << "Unable to initialize allegro primitives addon!" << endl;
        return;
    }
    if (!al_init_font_addon())
    {
        cerr << "Unable to initialize allegro font addon!" << endl;
        return;
    }
    if (!al_init_ttf_addon())
    {
        cerr << "Unable to initialize allegro ttf addon!" << endl;
        return;
    }
    if (!al_install_audio())
    {
        cerr << "Unable to initialize allegro audio!" << endl;
        return;
    }
    if (!al_init_acodec_addon())
    {
        cerr << "Unable to initialize allegro codecs!" << endl;
        return;
    }


    //  bu fonksiyon nabiyo?
    al_reserve_samples(1);

 
    

    //  create event queue
    eventQueue = al_create_event_queue();
    if (!eventQueue)
    {
        cerr << "Unable to create allegro event queue!" << endl;
        return;
    }

    

    /*
    void al_flush_event_queue(ALLEGRO_EVENT_QUEUE *queue)
    */

    
    //  create screen
    al_set_new_display_flags(ALLEGRO_WINDOWED);
    display = al_create_display(width, height);
    if (!display)
    {
        cerr << "Unable to create display!" << endl;
        return;
    }



    //  register keyboard and mouse events to event queue
    al_register_event_source(eventQueue, al_get_keyboard_event_source());
    al_register_event_source(eventQueue, al_get_mouse_event_source());


    //  register display events
    al_register_event_source(eventQueue, al_get_display_event_source(display));


    //  DENEME BLENDER
    //al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);



    //  load font(s)

    


    
    
    //  set screen window title
    
    this->fullScreen = fullScreen;
    this->running = true;


    //  at start, singleplayer is true and multiplayer is false
    this->singlePlayer = true;
    this->multiPlayer = false;
    
    cout << "GameEngine Init" << endl;

    //std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;


}

void GameEngine::Cleanup()
{
    //  cleanup all states
    while (!states.empty())
    {
        states.back()->Cleanup();
        states.pop_back();
    }
    
    //  switch back to windowed mode for somehow
    
    
    cout << "GameEngine Cleanup" << endl;
    
    
    //  shutdown allegro
}

void GameEngine::ChangeState(GameState* state)
{
    //  cleanup current state
    if (!states.empty())
    {
        states.back()->Cleanup();
        states.pop_back();
    }
    
    //  store and init new state
    states.push_back(state);
    states.back()->Init();

    
    //  for now do this here
    //TODO: think about this
    al_stop_timer(this->mainTimer);
    al_set_timer_speed(this->mainTimer, 1.0 / GetCurrentState()->mainThreadFPS);
    al_start_timer(this->mainTimer);
}

void GameEngine::PushState(GameState* state)
{
    //  pause current state
    if (!states.empty())
    {
        states.back()->Pause();
    }
    
    //  store and init new state
    states.push_back(state);
    states.back()->Init();
}

void GameEngine::PopState()
{
    //  cleanup the current state
    if (!states.empty())
    {
        states.back()->Cleanup();
        states.pop_back();
    }
    
    //  resume previous state
    if (!states.empty())
    {
        states.back()->Resume();
    }
}

void GameEngine::HandleEvents()
{
    //  let the state handl events
    states.back()->HandleEvents(this);
}

void GameEngine::Update()
{
    //  let the state update the game
    states.back()->Update(this);
}

void GameEngine::Draw()
{
    //  let the state draw the screen
    states.back()->Draw(this);
}
    
GameState* GameEngine::GetCurrentState()
{
    if (!states.empty())
    {
        return states.back();
    }
    else
    {
        return NULL;
    }
   
}