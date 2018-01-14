//
// Created by cpasjuste on 14/01/18.
//

#ifndef GUI_ROMIST_H
#define GUI_ROMIST_H

#include "gui.h"

class Config;

class Option;

class GuiRomInfo;

class Gui;

class GuiRomList : public c2d::Rectangle {

public:

    GuiRomList(Gui *gui, const c2d::Vector2f &size);

    ~GuiRomList();

    int updateState();

private:

    void filterRoms();

    Gui *gui = NULL;
    c2d::ListBox *list_box = NULL;
    GuiRomInfo *rom_info = NULL;

    RomList *rom_list = NULL;
    std::vector<RomList::Rom *> roms;
    RomList::Rom *rom = NULL;
    int rom_index = 0;

    int title_loaded = 0;
    int title_delay = 0;

    Timer *timer_input = NULL;
    Timer *timer_load = NULL;
};

#endif //GUI_ROMIST_H
