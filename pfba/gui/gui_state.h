//
// Created by cpasjuste on 16/02/18.
//

#ifndef PFBA_GUI_STATES_H
#define PFBA_GUI_STATES_H

#include "gui.h"

class GUISaveStateList;

class GuiState : public c2d::Rectangle {

public:

    GuiState(Gui *ui);

    void load();

    void unload();

    int update();

private:

    Gui *ui = NULL;
    GUISaveStateList *uiStateList = NULL;
    c2d::Text *title = NULL;
    bool isEmuRunning = false;
};

#endif //PFBA_GUI_STATES_H
