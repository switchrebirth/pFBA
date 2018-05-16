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
//extern int nSekCpuCore;

GuiEmu::GuiEmu(Gui *g) : Rectangle(g->getRenderer()->getSize()) {

    ui = g;
    setFillColor(Color::Transparent);

    fpsText = new Text("0123456789", *ui->getSkin()->font, (unsigned int) ui->getFontSize());
    fpsText->setString("FPS: 00/60");
    fpsText->setPosition(16, 16);
    add(fpsText);

    setVisibility(Hidden);
}

int GuiEmu::run(int driver) {

    bForce60Hz = true;
/*
#if defined(__PSP2__) || defined(__RPI__)
    nSekCpuCore = getSekCpuCore();
#endif
*/
    ///////////
    // AUDIO
    //////////
    int audio = ui->getConfig()->getValue(Option::Index::ROM_AUDIO, true);
    if (audio && ui->getAudio()->available) {
        printf("Creating audio device\n");
        // disable interpolation as it produce "cracking" sound
        // on some games (cps1 (SF2), cave ...)
        nInterpolation = 1;
        nFMInterpolation = 0;
        nBurnSoundRate = ui->getAudio()->frequency;
        nBurnSoundLen = ui->getAudio()->buffer_len;
        pBurnSoundOut = ui->getAudio()->buffer;
        ui->getAudio()->reset();
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
        ui->getUiProgressBox()->setVisibility(Hidden);
        ui->getUiMessageBox()->show("ERROR", "DRIVER INIT FAILED", "OK");
        return -1;
    }

    ///////////
    // VIDEO
    //////////
    printf("Creating video device\n");
    ui->getRenderer()->clear();
    int w, h;
    BurnDrvGetFullSize(&w, &h);
    video = new Video(ui, Vector2f(w, h));
    add(video);
    // set fps text on top
    fpsText->setLayer(1);

    setVisibility(Visible);
    ui->getUiProgressBox()->setVisibility(Hidden);
    ui->getUiRomList()->setVisibility(Hidden);

    // set per rom input configuration
    ui->updateInputMapping(true);

    // reset
    paused = false;
    nFramesEmulated = 0;
    nFramesRendered = 0;
    nCurrentFrame = 0;
    frame_duration = 1.0f / ((float) nBurnFPS / 100.0f);

    return 0;
}

void GuiEmu::stop() {

    DrvExit();
    InpExit();

    if (ui->getAudio()) {
        ui->getAudio()->pause(1);
    }

    if (video) {
        delete (video);
        video = NULL;
    }

    ui->updateInputMapping(false);
    setVisibility(Hidden);
}

void GuiEmu::pause() {

    paused = true;
    if (ui->getAudio()) {
        ui->getAudio()->pause(1);
    }
    ui->updateInputMapping(false);
#ifdef __NX__
    // restore ui rotation and scaling
    video->clear();
    gfxSetMode(GfxMode_TiledDouble);
    gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_V);
    gfxConfigureResolution(0, 0);
#endif
}

void GuiEmu::resume() {

    ui->updateInputMapping(true);
    if (ui->getAudio()) {
        ui->getAudio()->pause(0);
    }
    paused = false;
    ui->getRenderer()->clear();
#ifdef __NX__
    // restore game rotation and scaling
    video->updateScaling();
#endif
}

void GuiEmu::updateFramebuffer() {

    if (pBurnDraw == NULL) {
        nFramesEmulated++;
        nCurrentFrame++;
        nFramesRendered++;
        video->lock(NULL, (void **) &pBurnDraw, &nBurnPitch);
        BurnDrvFrame();
        video->unlock();
    }
}

void GuiEmu::renderFrame(bool bDraw, int bDrawFps, float fps) {

    fpsText->setVisibility(
            bDrawFps ? Visible : Hidden);

    if (!paused) {

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
        }

        if (bDrawFps) {
            sprintf(fpsString, "FPS: %.2g/%2d", fps, (nBurnFPS / 100));
            fpsText->setString(fpsString);
        }

        if (ui->getAudio() && ui->getAudio()->available) {
            ui->getAudio()->play();
        }
    }
}

float timer = 0;

void GuiEmu::updateFrame() {

    int showFps = ui->getConfig()->getValue(Option::Index::ROM_SHOW_FPS, true);
    int frameSkip = ui->getConfig()->getValue(Option::Index::ROM_FRAMESKIP, true);

    if (frameSkip) {
        bool draw = nFramesEmulated % (frameSkip + 1) == 0;
        renderFrame(draw, showFps, ui->getRenderer()->getFps());
#ifdef __NX__
        ui->getRenderer()->flip(false);
#else
        ui->getRenderer()->flip(draw);
#endif
        float delta = ui->getRenderer()->getDeltaTime().asSeconds();
        if (delta < frame_duration) { // limit fps
            //printf("f: %f | d: %f | m: %f | s: %i\n", frame_duration, delta, frame_duration - delta,
            //       (unsigned int) ((frame_duration - delta) * 1000));
            ui->getRenderer()->delay((unsigned int) ((frame_duration - delta) * 1000));
        }
    } else {
        renderFrame(true, showFps, ui->getRenderer()->getFps());
#ifdef __NX__
        ui->getRenderer()->flip(false);
#else
        ui->getRenderer()->flip();
#endif
        timer += ui->getRenderer()->getDeltaTime().asSeconds();
        if (timer >= 1) {
            timer = 0;
            printf("fps: %.2g/%2d, delta: %f\n", ui->getRenderer()->getFps(), (nBurnFPS / 100),
                   ui->getRenderer()->getDeltaTime().asSeconds());
        }
    }
}

int GuiEmu::update() {

    inputServiceSwitch = 0;
    inputP1P2Switch = 0;

    int rotation_config =
            ui->getConfig()->getValue(Option::Index::ROM_ROTATION, true);
    int rotate_input = 0;
#ifdef __PSP2__
    // TODO: find a way to unify platforms,
    // or allow rotation config from main.cpp
    if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        if (rotation_config == 0) {
            //rotate controls by 90 degrees
            rotate_input = 1;
        } else if (rotation_config == 2) {
            //rotate controls by 270 degrees
            rotate_input = 3;
        }
    }
#elif __SWITCH__
    if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        if (rotation_config == 0) {             // OFF
            //rotate controls by 270 degrees
            rotate_input = 3;
        } else if (rotation_config == 1) {      // ON
            //rotate controls by 270 degrees
            rotate_input = 0;
        } else if (rotation_config == 2) {      // FLIP
            //rotate controls by 270 degrees
            rotate_input = 1;
        }
    }
#else
    if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        rotate_input = rotation_config ? 0 : 3;
    }
#endif

    Input::Player *players = ui->getInput()->update(rotate_input);

    // process menu
    if ((players[0].state & Input::Key::KEY_START)
        && (players[0].state & Input::Key::KEY_COIN)) {
        pause();
        return UI_KEY_SHOW_MEMU_ROM;
    } else if ((players[0].state & Input::Key::KEY_START)
               && (players[0].state & Input::Key::KEY_FIRE5)) {
        pause();
        return UI_KEY_SHOW_MEMU_STATE;
    } else if ((players[0].state & Input::Key::KEY_START)
               && (players[0].state & Input::Key::KEY_FIRE3)) {
        inputServiceSwitch = 1;
    } else if ((players[0].state & Input::Key::KEY_START)
               && (players[0].state & Input::Key::KEY_FIRE4)) {
        inputP1P2Switch = 1;
    } else if (players[0].state & EV_RESIZE) {
        // useful for sdl resize event for example
        video->updateScaling();
    }

    InpMake(players);
    updateFrame();

    return 0;
}

Video *GuiEmu::getVideo() {
    return video;
}

/*
 * it seems cyclone asm core needs fixes to work with fba 0.2.97.43+
 * disable it for now as it's not really useful anyway...
 *
#if defined(__PSP2__) || defined(__RPI__)

int GuiEmu::getSekCpuCore() {

    int sekCpuCore = 0; // SEK_CORE_C68K: USE CYCLONE ARM ASM M68K CORE
    // int sekCpuCore = g->GetConfig()->GetRomValue(Option::Index::ROM_M68K);

    std::vector<std::string> zipList;
    int hardware = BurnDrvGetHardwareCode();

    if (!ui->getConfig()->getValue(Option::Index::ROM_NEOBIOS, true)
        && RomList::IsHardware(hardware, HARDWARE_PREFIX_SNK)) {
        sekCpuCore = 1; // SEK_CORE_M68K: USE C M68K CORE
        ui->getUiMessageBox()->show("WARNING", "UNIBIOS DOESNT SUPPORT THE M68K ASM CORE\n"
                "CYCLONE ASM CORE DISABLED", "OK");
    }

    if (RomList::IsHardware(hardware, HARDWARE_PREFIX_SEGA)) {
        if (hardware & HARDWARE_SEGA_FD1089A_ENC
            || hardware & HARDWARE_SEGA_FD1089B_ENC
            || hardware & HARDWARE_SEGA_MC8123_ENC
            || hardware & HARDWARE_SEGA_FD1094_ENC
            || hardware & HARDWARE_SEGA_FD1094_ENC_CPU2) {
            sekCpuCore = 1; // SEK_CORE_M68K: USE C M68K CORE
            ui->getUiMessageBox()->show("WARNING", "ROM IS CRYPTED, USE DECRYPTED ROM (CLONE)\n"
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
            ui->getUiMessageBox()->show("WARNING", "THIS ROM DOESNT SUPPORT THE M68K ASM CORE\n"
                    "CYCLONE ASM CORE DISABLED", "OK");
            sekCpuCore = 1; // SEK_CORE_M68K: USE C M68K CORE
            break;
        }
    }

    zipList.clear();

    return sekCpuCore;
}
#endif
*/
