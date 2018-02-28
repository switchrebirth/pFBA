//
// Created by cpasjuste on 23/11/16.
//

#include "skin.h"

#ifdef __NX__

#include "data/skin/default.h"
#include "data/skin/title.h"

#endif

using namespace c2d;

Skin::Skin(const std::string &path, const std::vector<Button> &btns) {

    this->path = path;

#ifdef __NX__
    //tex_bg = (Texture *) new C2DTexture("");
    tex_title = (Texture *) new C2DTexture((const unsigned char *) pfba_title, pfba_title_length);
    font = new C2DFont();
    font->loadFromMemory(pfba_font, pfba_font_length);
#else
    tex_bg = (Texture *) new C2DTexture((path + "/fba_bg.png").c_str());
    tex_title = (Texture *) new C2DTexture((path + "/title.png").c_str());
    font = new C2DFont();
    font->loadFromFile(path + "/default.ttf");
#endif

    // load buttons textures
    buttons = btns;
    char str[MAX_PATH];
    for (unsigned int i = 0; i < buttons.size(); i++) {
        memset(str, 0, MAX_PATH);
        snprintf(str, MAX_PATH, "%s/buttons/%i.png", path.c_str(), buttons[i].id);
        buttons[i].path.clear();
        buttons[i].path.append(str);
    }
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
    if (tex_bg)
        delete (tex_bg);
    if (tex_title)
        delete (tex_title);
}
