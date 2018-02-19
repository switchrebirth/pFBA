//
// Created by cpasjuste on 03/02/18.
//

#ifndef PFBA_GUI_EMU_H
#define PFBA_GUI_EMU_H

#include <sys/time.h>
#include "gui.h"
#include "video.h"
#include "skeleton/audio.h"

#ifdef __NX__
#define Video NXVideo
#endif

class GuiEmu : public c2d::Rectangle {

public:

    GuiEmu(Gui *gui);

    int run(int driver);

    void stop();

    void pause();

    void resume();

    int update();

    void renderFrame(bool draw = true, int drawFps = false, float fps = 0);

    Video *getVideo();

private:

    void updateFrame();

    Gui *ui = NULL;
    Video *video = NULL;
    c2d::Audio *audio = NULL;
    c2d::Text *fpsText = NULL;
    char fpsString[32];
    bool paused = false;
    float frame_duration = 0;

#if defined(__PSP2__) || defined(__RPI__)

    int getSekCpuCore();

#endif

};

#endif //PFBA_GUI_EMU_H
