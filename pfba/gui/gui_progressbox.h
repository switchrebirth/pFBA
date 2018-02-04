//
// Created by cpasjuste on 04/02/18.
//

#ifndef PFBA_GUI_PROGRESSBOX_H
#define PFBA_GUI_PROGRESSBOX_H

#include "skeleton/rectangle.h"
#include "gui.h"

class GuiProgressBox : public c2d::Rectangle {

public:

    GuiProgressBox(Gui *gui);

    void setTitle(std::string title);

    void setMessage(std::string message);

    void setProgress(float progress);

private:

    c2d::Text *title;
    c2d::Text *message;
    c2d::Rectangle *progress_bg;
    c2d::Rectangle *progress_fg;
};

#endif //PFBA_GUI_PROGRESSBOX_H
