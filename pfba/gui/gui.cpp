//
// Created by cpasjuste on 22/11/16.
//
#include <skeleton/timer.h>
#include <algorithm>
#include "run.h"
#include "video.h"
#include "menu.h"

using namespace c2d;

extern Video *video;

extern INT32 MakeScreenShot(const char *dst);

Gui::Gui(Io *i, Renderer *r, Skin *s, Config *cfg, Input *in) {

    io = i;
    renderer = r;
    skin = s;
    config = cfg;
    input = in;

    // build menus from options
    menu_gui = new Menu(NULL, cfg->getOptions());
    menu_rom = new Menu(NULL, cfg->getOptions(true), true);
    menu_current = menu_gui;

    // scaling factor mainly used for borders
    // based on vita resolution..
    scaling = std::min(renderer->getSize().x / 960, 1.0f);

    uiRomList = new GuiRomList(this, renderer->getSize());
    uiRomList->setLoadDelay(500);

    renderer->add(uiRomList);
}

Gui::~Gui() {

    delete (menu_gui);
    delete (menu_rom);
}

void Gui::run() {

    updateInputMapping(false);

    while (!quit) {

        switch (uiRomList->updateKeys()) {

            case Input::Key::KEY_FIRE1:
                runRom(uiRomList->getRom());
                break;

            case Input::Key::KEY_MENU1:
                // TODO: run option menu
                //RunOptionMenu();
                break;

            case Input::Key::KEY_MENU2:
                // TODO: run option menu (per rom options)
                getConfig()->load(uiRomList->getRom());
                //RunOptionMenu(true);
                break;

            default:
                break;
        }

        renderer->flip();
    }

}

float Gui::getScaling() {
    return scaling;
}

void Gui::runRom(RomList::Rom *rom) {

    if (rom == NULL) {
        return;
    }

    char path[MAX_PATH];
    for (int i = 0; i < DIRS_MAX; i++) {
        if (strlen(config->getRomPath(i)) > 0) {
            sprintf(path, "%s%s.zip", config->getRomPath(i), rom->zip);
            printf("%s\n", path);
            if (io->Exist(path))
                break;
        }
    }

    if (!io->Exist(path)) {
        printf("RunRom: rom not found: `%s`\n", rom->zip);
        return;
    }

    printf("RunRom: %s\n", path);
    for (nBurnDrvSelect[0] = 0; nBurnDrvSelect[0] < nBurnDrvCount; nBurnDrvSelect[0]++) {
        nBurnDrvActive = nBurnDrvSelect[0];
        if (strcasecmp(rom->zip, BurnDrvGetTextA(DRV_NAME)) == 0)
            break;
    }

    if (nBurnDrvActive >= nBurnDrvCount) {
        printf("RunRom: driver not found\n");
        return;
    }

    // load rom settings
    printf("RunRom: config->LoadRom(%s)\n", rom->zip);
    config->load(rom);

    // set per rom input scheme
    updateInputMapping(true);

    printf("RunRom: RunEmulator: start\n");
    RunEmulator(this, nBurnDrvActive);

    // set default input scheme
    updateInputMapping(false);

    printf("RunRom: RunEmulator: return\n");
}

Input *Gui::getInput() {
    return input;
}

Renderer *Gui::getRenderer() {
    return renderer;
}

Skin *Gui::getSkin() {
    return skin;
}

Config *Gui::getConfig() {
    return config;
}

Io *Gui::getIo() {
    return io;
}

void Gui::updateInputMapping(bool isRomConfig) {

    if (isRomConfig) {
        input->SetKeyboardMapping(config->getRomPlayerInputKeys(0));
        int dz = 2000 + config->getValue(Option::Index::JOY_DEADZONE, true) * 2000;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            input->SetJoystickMapping(i, config->getRomPlayerInputButtons(i), dz);
            input->players[i].lx.id = config->getValue(Option::Index::JOY_AXIS_LX, true);
            input->players[i].ly.id = config->getValue(Option::Index::JOY_AXIS_LY, true);
            input->players[i].rx.id = config->getValue(Option::Index::JOY_AXIS_RX, true);
            input->players[i].ry.id = config->getValue(Option::Index::JOY_AXIS_RY, true);
        }
    } else {
        input->SetKeyboardMapping(config->getGuiPlayerInputKeys(0));
        int dz = 2000 + config->getValue(Option::Index::JOY_DEADZONE) * 2000;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            input->SetJoystickMapping(i, config->getGuiPlayerInputButtons(i), dz);
            input->players[i].lx.id = config->getValue(Option::Index::JOY_AXIS_LX);
            input->players[i].ly.id = config->getValue(Option::Index::JOY_AXIS_LY);
            input->players[i].rx.id = config->getValue(Option::Index::JOY_AXIS_RX);
            input->players[i].ry.id = config->getValue(Option::Index::JOY_AXIS_RY);
        }
    }
}
