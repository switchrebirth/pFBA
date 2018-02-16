//
// Created by cpasjuste on 03/02/18.
//

#ifndef PFBA_GUI_EMU_H
#define PFBA_GUI_EMU_H

#include <sys/time.h>
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

    void renderFrame(bool draw, int drawFps, float fps);

    Video *getVideo();

private:

    void updateFrame();

    Gui *gui = NULL;
    Video *video = NULL;
    c2d::Audio *audio = NULL;
    c2d::Text *fpsText = NULL;
    char fpsString[32];

    float frame_time, time_now, time_last, fps;

#if defined(__PSP2__) || defined(__RPI__)

    int getSekCpuCore();

#endif

};


#endif //PFBA_GUI_EMU_H
