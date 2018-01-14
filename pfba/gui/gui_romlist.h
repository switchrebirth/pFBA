//
// Created by cpasjuste on 14/01/18.
//

#ifndef GUI_ROMIST_H
#define GUI_ROMIST_H

#include "gui.h"

class Config;

class Option;

class GuiRomInfo;

class GuiRomList : public c2d::Rectangle {

public:

    GuiRomList(Gui *gui);

    ~GuiRomList();

private:

    void filterRoms();

    Menu *menu_gui = NULL;
    Menu *menu_rom = NULL;
    Menu *menu_current = NULL;

    c2d::ListBox *list_box = NULL;
    GuiRomInfo *rom_info = NULL;

    RomList *rom_list = NULL;
    std::vector<RomList::Rom *> roms;
    RomList::Rom *rom_selected = NULL;

    float scaling = 1;
};

#endif //GUI_ROMIST_H
