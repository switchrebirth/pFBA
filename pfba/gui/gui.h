//
// Created by cpasjuste on 14/01/18.
//

#ifndef GUI_H
#define GUI_H

#include <c2d.h>

#include "skin.h"
#include "romlist.h"
#include "config.h"
#include "option_menu.h"

class Config;

class Option;

class GuiMenu;

class GuiRomList;

class GuiRomInfo;

class GuiEmu;

class GuiState;

class GuiProgressBox;

#define UI_KEY_FILTER_ROMS      100
#define UI_KEY_SHOW_ROMLIST     101
#define UI_KEY_SHOW_MEMU_UI     102
#define UI_KEY_SHOW_MEMU_ROM    103
#define UI_KEY_SHOW_MEMU_STATE  104
#define UI_KEY_RUN_ROM          105
#define UI_KEY_RESUME_ROM       106
#define UI_KEY_STOP_ROM         107

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

    GuiRomList *getUiRomList();

    GuiEmu *getUiEmu();

    GuiMenu *getUiMenu();

    GuiProgressBox *getUiProgressBox();

    c2d::MessageBox *getUiMessageBox();

    c2d::Font *getFont();

    int getFontSize();

    float getScaling();

private:

    c2d::Io *io = NULL;
    Config *config = NULL;
    c2d::Renderer *renderer = NULL;
    Skin *skin = NULL;
    c2d::Input *input = NULL;

    GuiMenu *uiMenu = NULL;
    GuiRomList *uiRomList = NULL;
    GuiEmu *uiEmu = NULL;
    GuiState *uiState = NULL;
    GuiProgressBox *uiProgressBox = NULL;
    c2d::MessageBox *uiMessageBox = NULL;

    float scaling = 1;
};

#endif //GUI_MAIN_H
