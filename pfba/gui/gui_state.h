//
// Created by cpasjuste on 16/02/18.
//

#ifndef PFBA_GUI_STATES_H
#define PFBA_GUI_STATES_H

#include "gui.h"

class GUISaveState;

class GuiState : public c2d::Rectangle {

public:

    GuiState(Gui *ui);

    void load();

    void unload();

    void setSelection(int index);

    int update();

private:

    Gui *ui;
    GUISaveState *states[4];
    int state_index = 0;
    bool isEmuRunning = false;
};

#endif //PFBA_GUI_STATES_H
