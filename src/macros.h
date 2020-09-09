#ifndef MACROS_H
#define MACROS_H



#include <allegro5/allegro.h>


#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600


#define GAME_INTRO_FPS 30
#define GAME_MENU_FPS 30
#define GAME_FPS 30
#define GAME_MULTIPLAYER_FPS GAME_FPS


#define COLOR_WHITE al_map_rgba(255, 255, 255, 255)
#define COLOR_BLACK al_map_rgba(0, 0, 0, 255)


#define COLOR_RED al_map_rgba(255, 0, 0, 255)
#define COLOR_BLUE al_map_rgba(0, 0, 255, 255)
#define COLOR_TEAL al_map_rgba(0, 255, 255, 255)
#define COLOR_PURPLE al_map_rgba(255, 0, 255, 255)

#define COLOR_GREEN al_map_rgba(0, 255, 0, 255)


#define SNAKE_WIDTH 5
#define SNAKE_HEIGHT 5
#define SNAKE_SIZE 5  //  square size, two definitions above are invalid
#define SNAKE_SPEED 12



#define DIRECTION_RIGHT 0
#define DIRECTION_UP 1
#define DIRECTION_LEFT 2
#define DIRECTION_DOWN 3

#define DIRECTION_SAME 10  //  message for not changing direction


//#define CONFIG_FILENAME "config.ini"


#endif