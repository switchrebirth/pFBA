//
// Created by cpasjuste on 14/01/18.
//

#ifndef GUI_H
#define GUI_H

#include <c2d.h>

#include "skin.h"
#include "romlist.h"
#include "config.h"
#include "menu.h"
#include "gui_romlist.h"

class Config;

class Option;

//class GuiRomList;

class GuiRomInfo;

#define UI_MARGIN   16
#define INPUT_DELAY 150

#define COL_GREEN   Color(153, 255, 51)
#define COL_YELLOW  Color(255, 225, 51)
#define COL_ORANGE  Color(255, 153, 51)
#define COL_RED     Color(255, 51, 51)

class Gui {

public:

    Gui(c2d::Io *io, c2d::Renderer *rdr,
        Skin *skin, Config *cfg, c2d::Input *input);

    ~Gui();

    void run();

    void runRom(RomList::Rom *rom);

    int getButton();

    void updateInputMapping(bool isRomCfg);

    c2d::Renderer *getRenderer();

    Skin *getSkin();

    Config *getConfig();

    c2d::Input *getInput();

    c2d::Io *getIo();

    float getScaling();

private:

    bool IsOptionHidden(Option *option);

    c2d::Io *io = NULL;
    Config *config = NULL;
    c2d::Renderer *renderer = NULL;
    Skin *skin = NULL;
    c2d::Input *input = NULL;

    Menu *menu_gui = NULL;
    Menu *menu_rom = NULL;
    Menu *menu_current = NULL;

    GuiRomList *uiRomList;

    float scaling = 1;

    bool quit = false;
};

#endif //GUI_MAIN_H
