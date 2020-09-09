#include "menubutton.h"

MenuButton::MenuButton(std::string buttonString, int x, int y, int width, int height,
               ALLEGRO_FONT* buttonFont,
               ALLEGRO_COLOR defaultTextColor,
               ALLEGRO_COLOR hoverTextColor,
               ALLEGRO_COLOR defaultEdgeColor,
               ALLEGRO_COLOR defaultInnerColor,
               ALLEGRO_COLOR hoverEdgeColor,
               ALLEGRO_COLOR hoverInnerColor)
{
    this->buttonString = buttonString;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->buttonFont = buttonFont;
    this->defaultTextColor = defaultTextColor;
    this->hoverTextColor = hoverTextColor;
    this->defaultEdgeColor = defaultEdgeColor;
    this->defaultInnerColor = defaultInnerColor;
    this->hoverEdgeColor = hoverEdgeColor;
    this->hoverInnerColor = hoverInnerColor;
}

void MenuButton::Draw(bool hovered)
{
    //  topleft point
    int x1 = this->x;
    int y1 = this->y;

    //  botright point
    int x2 = x1 + this->width;
    int y2 = y1 + this->height;

    if (!hovered)
    {
        al_draw_filled_rectangle(x1, y1, x2, y2, this->defaultInnerColor);
        al_draw_rectangle(x1, y1, x2, y2, this->defaultEdgeColor, 3);
        al_draw_text(this->buttonFont, this->defaultTextColor, int((x1+x2)/2), int((y1+y2)/2), ALLEGRO_ALIGN_CENTER, this->buttonString.c_str() );
    }
    else
    {
        al_draw_filled_rectangle(x1, y1, x2, y2, this->hoverInnerColor);
        al_draw_rectangle(x1, y1, x2, y2, this->hoverEdgeColor, 3);
        al_draw_text(this->buttonFont, this->hoverTextColor, int((x1+x2)/2), int((y1+y2)/2), ALLEGRO_ALIGN_CENTER, this->buttonString.c_str() );
    }
    //al_draw_text(mainmenu.buttonFont, buttonTextColor[2], gameWidth / 2 , mainmenu.y[2] + buttonFontSize, ALLEGRO_ALIGN_CENTER, "OPTIONS");
}

bool MenuButton::IsPointInsideButton(int px, int py)
{
    //  topleft point
    int x1 = this->x;
    int y1 = this->y;

    //  botright point
    int x2 = x1 + this->width;
    int y2 = y1 + this->height;

    return ( px >= x1 && px <= x2 && py >= y1 && py <= y2);
}