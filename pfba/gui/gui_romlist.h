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

    int update();

    void updateRomList();

    RomList::Rom *getSelection();

    void setLoadDelay(int delay);

private:

    Gui *ui = NULL;
    c2d::ListBox *list_box = NULL;
    GuiRomInfo *rom_info = NULL;

    RomList *rom_list = NULL;
    std::vector<RomList::Rom *> roms;
    int rom_index = 0;

    int title_loaded = 0;
    int load_delay = 100;
    c2d::C2DClock timer_input;
    c2d::C2DClock timer_load;
};

#endif //GUI_ROMIST_H
