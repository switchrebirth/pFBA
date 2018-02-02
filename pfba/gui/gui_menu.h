//
// Created by cpasjuste on 30/01/18.
//

#ifndef PFBA_GUI_MENU_H
#define PFBA_GUI_MENU_H

#include "skeleton/rectangle.h"
#include "gui.h"

class MenuLine;

class GuiMenu : public c2d::Rectangle {

public:

    GuiMenu(Gui *gui);

    ~GuiMenu();

    int updateKeys();

    void loadMenu(bool isRomMenu = false, OptionMenu *om = NULL);

private:

    bool isOptionHidden(Option *option);

    Gui *gui;
    c2d::Text *title;
    c2d::Rectangle *highlight;
    std::vector<MenuLine *> lines;

    OptionMenu *optionMenuGui;
    OptionMenu *optionMenuRom;
    OptionMenu *optionMenu;
    std::vector<Option> *options;
    int optionIndex = 0;
    int optionCount = 0;
    bool isRomMenu = false;
    bool isEmuRunning = false;
};

#endif //PFBA_GUI_MENU_H
