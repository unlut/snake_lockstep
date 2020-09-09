#ifndef MENUBUTTON_H
#define MENUBUTTON_H

#include <iostream>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include "macros.h"



class MenuButton
{
private:
    std::string buttonString;
    int x, y;
    int width, height;
    ALLEGRO_FONT* buttonFont;  //  maybe separate fonts for states (hover...)
    ALLEGRO_COLOR defaultTextColor;
    ALLEGRO_COLOR hoverTextColor;
    ALLEGRO_COLOR defaultEdgeColor;
    ALLEGRO_COLOR defaultInnerColor;
    ALLEGRO_COLOR hoverEdgeColor;
    ALLEGRO_COLOR hoverInnerColor;

    //  maybe button bitmap?

    
    
public:
    MenuButton(std::string buttonString, int x, int y, int width, int height,
               ALLEGRO_FONT* buttonFont,
               ALLEGRO_COLOR defaultTextColor,
               ALLEGRO_COLOR hoverTextColor,
               ALLEGRO_COLOR defaultEdgeColor,
               ALLEGRO_COLOR defaultInnerColor,
               ALLEGRO_COLOR hoverEdgeColor,
               ALLEGRO_COLOR hoverInnerColor);

    void Draw(bool hovered=false);

    bool IsPointInsideButton(int px, int py);

};


#endif