//
// Created by cpasjuste on 03/02/18.
//

#include "gui_emu.h"
#include "gui_romlist.h"
#include "gui_progressbox.h"

using namespace c2d;

extern int InpMake(Input::Player *players);

extern unsigned char inputServiceSwitch;
extern unsigned char inputP1P2Switch;
extern int nSekCpuCore;
bool bPauseOn = false;

GuiEmu::GuiEmu(Gui *g) : Rectangle(g->getRenderer()->getSize()) {

    gui = g;
    setFillColor(Color::Transparent);

    fpsText = new Text("0123456789", *gui->getSkin()->font, (unsigned int) gui->getFontSize());
    fpsText->setPosition(16, 16);
    add(fpsText);
}

int GuiEmu::load(int driver) {

    bForce60Hz = true;
#if defined(__PSP2__) || defined(__RPI__)
    nSekCpuCore = getSekCpuCore();
#endif
    ///////////
    // AUDIO
    //////////
    nBurnSoundRate = 0;
    if (gui->getConfig()->getValue(Option::Index::ROM_AUDIO, true)) {
#ifdef __NX__
        nBurnSoundRate = 0;
#elif __3DS__
        nBurnSoundRate = 44100;
#else
        nBurnSoundRate = 48000;
#endif
    }
    if (nBurnSoundRate > 0) {
        printf("Creating audio device\n");
        // disable interpolation as it produce "cracking" sound
        // on some games (cps1 (SF2), cave ...)
        nInterpolation = 1;
        nFMInterpolation = 0;
        audio = new C2DAudio(nBurnSoundRate, nBurnFPS);
        if (audio->available) {
            nBurnSoundRate = audio->frequency;
            nBurnSoundLen = audio->buffer_len;
            pBurnSoundOut = audio->buffer;
        }
    }

    if (!audio || !audio->available) {
        printf("Audio disabled\n");
        nBurnSoundRate = 0;
        nBurnSoundLen = 0;
        pBurnSoundOut = NULL;
    }

    ///////////
    // DRIVER
    //////////
    InpInit();
    InpDIP();

    printf("Initialize driver\n");
    if (DrvInit(driver, false) != 0) {
        printf("Driver initialisation failed! Likely causes are:\n"
                       "- Corrupt/Missing ROM(s)\n"
                       "- I/O Error\n"
                       "- Memory error\n\n");
        DrvExit();
        InpExit();
        if (audio) {
            delete (audio);
        }
        gui->getUiProgressBox()->setVisibility(C2D_VISIBILITY_HIDDEN);
        gui->getUiMessageBox()->show("ERROR", "DRIVER INIT FAILED", "OK");
        return -1;
    }

    ///////////
    // VIDEO
    //////////
    printf("Creating video device\n");
    int w, h;
    BurnDrvGetFullSize(&w, &h);
    video = new Video(gui, Vector2f(w, h));
    add(video);
    // set fps text on top
    fpsText->setLayer(1);

    // reset
    bPauseOn = false;
    nFramesEmulated = 0;
    nCurrentFrame = 0;
    nFramesRendered = 0;
    startTicks();

    setVisibility(C2D_VISIBILITY_VISIBLE);
    gui->getUiProgressBox()->setVisibility(C2D_VISIBILITY_HIDDEN);
    gui->getUiRomList()->setVisibility(C2D_VISIBILITY_HIDDEN);

    // set per rom input configuration
    gui->updateInputMapping(true);

    return 0;
}

void GuiEmu::unload() {

    DrvExit();
    InpExit();
    if (video) {
        delete (video);
        video = NULL;
    }
    if (audio) {
        delete (audio);
        audio = NULL;
    }

    setVisibility(C2D_VISIBILITY_HIDDEN);
}

void GuiEmu::pause() {

    bPauseOn = true;
    if (audio) {
        audio->Pause(1);
    }
    gui->updateInputMapping(false);
}

void GuiEmu::resume() {

    gui->updateInputMapping(true);
    if (audio) {
        audio->Pause(0);
    }
    bPauseOn = false;
}

void GuiEmu::renderFrame(bool bDraw, int bDrawFps, int fps) {

    fpsText->setVisibility(
            bDrawFps ? C2D_VISIBILITY_VISIBLE : C2D_VISIBILITY_HIDDEN);

    if (!bPauseOn) {

        nFramesEmulated++;
        nCurrentFrame++;

        pBurnDraw = NULL;
        if (bDraw) {
            nFramesRendered++;
            video->lock(NULL, (void **) &pBurnDraw, &nBurnPitch);
        }

        BurnDrvFrame();

        if (bDraw) {
            video->unlock();
            if (bDrawFps) {
                sprintf(fpsString, "FPS: %2d/%2d", fps, (nBurnFPS / 100));
                fpsText->setString(fpsString);
            }
        }
    }
}

void GuiEmu::updateFrame() {

    int showFps = gui->getConfig()->getValue(Option::Index::ROM_SHOW_FPS, true);
    int frameSkip = gui->getConfig()->getValue(Option::Index::ROM_FRAMESKIP, true);

    if (frameSkip) {
        timer = getTicks() / frametime;
        if (timer - tick > frame_limit && showFps) {
            fps = nFramesRendered;
            nFramesRendered = 0;
            tick = timer;
        }
        now = timer;
        ticks = now - done;
        if (ticks > 0) {
            if (ticks > 10) ticks = 10;
            for (int i = 0; i < ticks - 1; i++) {
                renderFrame(false, showFps, fps);
                if (audio) {
                    audio->Play();
                }
            }
            if (ticks >= 1) {
                renderFrame(true, showFps, fps);
                if (audio) {
                    audio->Play();
                }
            }
            done = now;
        }
    } else {
        if (showFps) {
            timer = getTicks();
            if (timer - tick > 1000000) {
                fps = nFramesRendered;
                nFramesRendered = 0;
                tick = timer;
            }
        }
        renderFrame(true, showFps, fps);
        if (audio) {
            audio->Play();
        }
    }
}

int GuiEmu::update() {

    updateFrame();

    inputServiceSwitch = 0;
    inputP1P2Switch = 0;

    int rotation = gui->getConfig()->getValue(Option::Index::ROM_ROTATION, true);
    int rotate = 0;
    if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        if (rotation == 0) {
            //rotate controls by 90 degrees
            rotate = 1;
        }
        if (rotation == 2) {
            //rotate controls by 270 degrees
            rotate = 3;
        }
    }

    Input::Player *players = gui->getInput()->update(rotate);

    // process menu
    if ((players[0].state & Input::Key::KEY_MENU1)
        && (players[0].state & Input::Key::KEY_MENU2)) {
#ifdef    FAST_EXIT
        gui->GetInput()->Clear(0);
        GameLooping = false;
#else
        pause();
        return UI_KEY_SHOW_MEMU_ROM;
#endif
    } else if ((players[0].state & Input::Key::KEY_MENU2)
               && (players[0].state & Input::Key::KEY_FIRE5)) {
        pause();
        // TODO: update for latest cross2d
        //gui->RunStatesMenu();
        // restore rom control scheme
    } else if ((players[0].state & Input::Key::KEY_MENU2)
               && (players[0].state & Input::Key::KEY_FIRE3)) {
        inputServiceSwitch = 1;
    } else if ((players[0].state & Input::Key::KEY_MENU2)
               && (players[0].state & Input::Key::KEY_FIRE4)) {
        inputP1P2Switch = 1;
    } else if ((players[0].state & Input::Key::KEY_MENU2)
               && (players[0].state & Input::Key::KEY_UP)) {
        int scaling = gui->getConfig()->getValue(Option::Index::ROM_SCALING, true) + 1;
        if (scaling <= 5) {
            int index = gui->getConfig()->getOptionPos(gui->getConfig()->getOptions(true),
                                                       Option::Index::ROM_SCALING);
            gui->getConfig()->getOptions(true)->at(index).value = scaling;
            video->updateScaling();
            gui->getRenderer()->delay(500);
        }
    } else if ((players[0].state & Input::Key::KEY_MENU2)
               && (players[0].state & Input::Key::KEY_DOWN)) {
        int scaling = gui->getConfig()->getValue(Option::Index::ROM_SCALING, true) - 1;
        if (scaling >= 0) {
            int index = gui->getConfig()->getOptionPos(gui->getConfig()->getOptions(true),
                                                       Option::Index::ROM_SCALING);
            gui->getConfig()->getOptions(true)->at(index).value = scaling;
            video->updateScaling();
            gui->getRenderer()->delay(500);
        }
    } else if ((players[0].state & Input::Key::KEY_MENU2)
               && (players[0].state & Input::Key::KEY_RIGHT)) {
        int shader = gui->getConfig()->getValue(Option::Index::ROM_SHADER, true) + 1;
        if (shader < gui->getRenderer()->getShaderList()->getCount()) {
            int index = gui->getConfig()->getOptionPos(gui->getConfig()->getOptions(true),
                                                       Option::Index::ROM_SHADER);
            gui->getConfig()->getOptions(true)->at(index).value = shader;
            video->setShader(shader);
            gui->getRenderer()->delay(500);
        }
    } else if ((players[0].state & Input::Key::KEY_MENU2)
               && (players[0].state & Input::Key::KEY_LEFT)) {
        int shader = gui->getConfig()->getValue(Option::Index::ROM_SHADER, true) - 1;
        if (shader >= 0) {
            int index = gui->getConfig()->getOptionPos(gui->getConfig()->getOptions(true),
                                                       Option::Index::ROM_SHADER);
            gui->getConfig()->getOptions(true)->at(index).value = shader;
            video->setShader(shader);
            gui->getRenderer()->delay(500);
        }
    } else if (players[0].state & EV_RESIZE) {
        video->updateScaling();
    }

    InpMake(players);

    return 0;
}

void GuiEmu::startTicks() {

    frame_limit = nBurnFPS / 100;
    frametime = 100000000 / nBurnFPS;
    now = done = timer = tick = ticks = fps = 0;

    gettimeofday(&start, NULL);
}

unsigned int GuiEmu::getTicks() {
    unsigned int ticks;
    struct timeval n = {};
    gettimeofday(&n, NULL);
    ticks = (unsigned int) ((n.tv_sec - start.tv_sec) * 1000000 + n.tv_usec - start.tv_usec);
    return ticks;
}

Video *GuiEmu::getVideo() {
    return video;
}

#if defined(__PSP2__) || defined(__RPI__)

int GuiEmu::getSekCpuCore() {

    int sekCpuCore = 0; // SEK_CORE_C68K: USE CYCLONE ARM ASM M68K CORE
    // int sekCpuCore = g->GetConfig()->GetRomValue(Option::Index::ROM_M68K);

    std::vector<std::string> zipList;
    int hardware = BurnDrvGetHardwareCode();

    if (!gui->getConfig()->getValue(Option::Index::ROM_NEOBIOS, true)
        && RomList::IsHardware(hardware, HARDWARE_PREFIX_SNK)) {
        sekCpuCore = 1; // SEK_CORE_M68K: USE C M68K CORE
        gui->getUiMessageBox()->show("WARNING", "UNIBIOS DOESNT SUPPORT THE M68K ASM CORE\n"
                "CYCLONE ASM CORE DISABLED", "OK");
    }

    if (RomList::IsHardware(hardware, HARDWARE_PREFIX_SEGA)) {
        if (hardware & HARDWARE_SEGA_FD1089A_ENC
            || hardware & HARDWARE_SEGA_FD1089B_ENC
            || hardware & HARDWARE_SEGA_MC8123_ENC
            || hardware & HARDWARE_SEGA_FD1094_ENC
            || hardware & HARDWARE_SEGA_FD1094_ENC_CPU2) {
            sekCpuCore = 1; // SEK_CORE_M68K: USE C M68K CORE
            gui->getUiMessageBox()->show("WARNING", "ROM IS CRYPTED, USE DECRYPTED ROM (CLONE)\n"
                    "TO ENABLE CYCLONE ASM CORE (FASTER)", "OK");
        }
    } else if (RomList::IsHardware(hardware, HARDWARE_PREFIX_TOAPLAN)) {
        zipList.push_back("batrider");
        zipList.push_back("bbakraid");
        zipList.push_back("bgaregga");
    } else if (RomList::IsHardware(hardware, HARDWARE_PREFIX_SNK)) {
        zipList.push_back("kof97");
        zipList.push_back("kof98");
        zipList.push_back("kof99");
        zipList.push_back("kof2000");
        zipList.push_back("kof2001");
        zipList.push_back("kof2002");
        zipList.push_back("kf2k3pcb");
        //zipList.push_back("kof2003"); // WORKS
    }

    std::string zip = BurnDrvGetTextA(DRV_NAME);
    for (unsigned int i = 0; i < zipList.size(); i++) {
        if (zipList[i].compare(0, zip.length(), zip) == 0) {
            gui->getUiMessageBox()->show("WARNING", "THIS ROM DOESNT SUPPORT THE M68K ASM CORE\n"
                    "CYCLONE ASM CORE DISABLED", "OK");
            sekCpuCore = 1; // SEK_CORE_M68K: USE C M68K CORE
            break;
        }
    }

    zipList.clear();

    return sekCpuCore;
}

#endif
