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

class Gui : public c2d::Rectangle {

public:

    Gui(c2d::Io *io, c2d::Renderer *rdr, Skin *skin, RomList *rList, Config *cfg, c2d::Input *input);

    ~Gui();

    void Run();

    void RunRom(RomList::Rom *rom);

    void DrawOptions(bool isRomCfg, Menu *_menu);

    void DrawRomInfo(RomList::Rom *rom);

    void RunOptionMenu(bool isRomConfig = false);

    void RunStatesMenu();

    int GetButton();

    int TitleLoad(RomList::Rom *rom);

    void TitleFree();

    void SetTitleLoadDelay(int delay);

    void UpdateInputMapping(bool isRomCfg);

    void Clear();

    void Flip();

    c2d::Renderer *GetRenderer();

    Skin *GetSkin();

    Config *GetConfig();

    c2d::Input *GetInput();

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

    //GuiRomList *guiRomList = NULL;
    c2d::ListBox *guiRomList = NULL;
    GuiRomInfo *guiRomInfo = NULL;

    RomList *romList = NULL;
    std::vector<RomList::Rom *> roms;
    RomList::Rom *romSelected = NULL;

    float scaling = 1;
    int title_delay = 0;

    void FilterRoms();

    bool quit = false;
};

#endif //GUI_MAIN_H
