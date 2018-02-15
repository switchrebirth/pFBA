//
// Created by cpasjuste on 23/11/16.
//

#include "skin.h"
#include "burner.h"

#ifdef __NX__

#include "data/skin/default.h"
#include "data/skin/title.h"

#endif

using namespace c2d;

Skin::Skin(char *skinPath, std::vector<Button> btns) {

#ifdef __NX__
    tex_bg = (Texture *) new C2DTexture("");
    tex_title = (Texture *) new C2DTexture((const unsigned char *) pfba_title, pfba_title_length);
    font = new C2DFont();
    font->loadFromMemory(pfba_font, pfba_font_length);
    buttons = btns;
#else
    char str[MAX_PATH];
    memset(str, 0, MAX_PATH);
    snprintf(str, MAX_PATH, "%s/fba_bg.png", skinPath);
    tex_bg = (Texture *) new C2DTexture(str);

    memset(str, 0, MAX_PATH);
    snprintf(str, MAX_PATH, "%s/title.png", skinPath);
    tex_title = (Texture *) new C2DTexture(str);

    memset(str, 0, MAX_PATH);
    snprintf(str, MAX_PATH, "%s/default.ttf", skinPath);
    font = new C2DFont();
    font->loadFromFile(str);

    // load buttons textures
    buttons = btns;
    for (unsigned int i = 0; i < buttons.size(); i++) {
        memset(str, 0, MAX_PATH);
        snprintf(str, MAX_PATH, "%s/buttons/%i.png", skinPath, buttons[i].id);
        buttons[i].texture = (Texture *) new C2DTexture(str);
    }
#endif
}

Skin::Button *Skin::getButton(int id) {

    for (unsigned int i = 0; i < buttons.size(); i++) {
        if (buttons[i].id == id) {
            return &buttons[i];
        }
    }
    return NULL;
}

Skin::~Skin() {
    if (font)
        delete (font);
}
