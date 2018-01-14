//
// Created by cpasjuste on 23/11/16.
//

#ifndef _SKIN_H_
#define _SKIN_H_

#include <c2d.h>

class Skin {

public:

    struct Button {
        std::string name;
        int id = -1;
        c2d::Texture *texture = NULL;

        Button(int i, const std::string &n) {
            id = i;
            name = n;
        }
    };

    Skin(c2d::Renderer *renderer, char *skinPath, int fontSize, std::vector<Button> btns);

    ~Skin();

    Button *GetButton(int id);

    c2d::Texture *tex_bg;
    c2d::Texture *tex_title;
    c2d::Font *font;
    int fontSize = C2D_DEFAULT_CHAR_SIZE;
    std::vector<Button> buttons;
};


#endif //_SKIN_H