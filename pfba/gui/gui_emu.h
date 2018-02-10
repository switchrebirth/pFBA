//
// Created by cpasjuste on 03/02/18.
//

#ifndef PFBA_GUI_EMU_H
#define PFBA_GUI_EMU_H

#include "gui.h"
#include "video.h"
#include "skeleton/audio.h"

class GuiEmu : public c2d::Rectangle {

public:

    GuiEmu(Gui *gui);

    int load(int driver);

    void unload();

    void pause();

    void resume();

    int update();

    void updateFrame();

    void renderFrame(bool draw, int drawFps, int fps);

    Video *getVideo();

private:

    Gui *gui = NULL;
    Video *video = NULL;
    c2d::Audio *audio = NULL;
    c2d::Text *fpsText = NULL;
    char fpsString[32];

    int frame_limit, frametime;
    int now, done, timer, tick, ticks, fps;

    struct timeval start;

    void startTicks();

    unsigned int getTicks();

#if defined(__PSP2__) || defined(__RPI__)

    int getSekCpuCore();

#endif

};


#endif //PFBA_GUI_EMU_H
