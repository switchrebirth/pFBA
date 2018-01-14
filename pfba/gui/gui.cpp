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
    menu_gui = new Menu(NULL, cfg->GetGuiOptions());
    menu_rom = new Menu(NULL, cfg->GetRomOptions(), true);
    menu_current = menu_gui;

    // scaling factor mainly used for borders
    // based on vita resolution..
    scaling = std::min(renderer->getSize().x / 960, 1.0f);

    uiRomList = new GuiRomList(this, renderer->getSize());
    renderer->add(uiRomList);
}

Gui::~Gui() {

    delete (menu_gui);
    delete (menu_rom);
}

void Gui::run() {

    updateInputMapping(false);

    while (!quit) {

        int key = uiRomList->updateState();

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
        if (strlen(config->GetRomPath(i)) > 0) {
            sprintf(path, "%s%s.zip", config->GetRomPath(i), rom->zip);
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
    config->Load(rom);

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
        input->SetKeyboardMapping(config->GetRomPlayerInputKeys(0));
        int dz = 2000 + config->GetRomValue(Option::Index::JOY_DEADZONE) * 2000;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            input->SetJoystickMapping(i, config->GetRomPlayerInputButtons(i), dz);
            input->players[i].lx.id = config->GetRomValue(Option::Index::JOY_AXIS_LX);
            input->players[i].ly.id = config->GetRomValue(Option::Index::JOY_AXIS_LY);
            input->players[i].rx.id = config->GetRomValue(Option::Index::JOY_AXIS_RX);
            input->players[i].ry.id = config->GetRomValue(Option::Index::JOY_AXIS_RY);
        }
    } else {
        input->SetKeyboardMapping(config->GetGuiPlayerInputKeys(0));
        int dz = 2000 + config->GetGuiValue(Option::Index::JOY_DEADZONE) * 2000;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            input->SetJoystickMapping(i, config->GetGuiPlayerInputButtons(i), dz);
            input->players[i].lx.id = config->GetGuiValue(Option::Index::JOY_AXIS_LX);
            input->players[i].ly.id = config->GetGuiValue(Option::Index::JOY_AXIS_LY);
            input->players[i].rx.id = config->GetGuiValue(Option::Index::JOY_AXIS_RX);
            input->players[i].ry.id = config->GetGuiValue(Option::Index::JOY_AXIS_RY);
        }
    }
}
