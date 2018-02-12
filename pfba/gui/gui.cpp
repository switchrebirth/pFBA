//
// Created by cpasjuste on 22/11/16.
//
#include <skeleton/timer.h>
#include <algorithm>
#include "gui_menu.h"
#include "gui_emu.h"
#include "gui_romlist.h"
#include "gui_progressbox.h"

using namespace c2d;

Gui::Gui(Io *i, Renderer *r, Skin *s, Config *cfg, Input *in) {

    io = i;
    renderer = r;
    skin = s;
    config = cfg;
    input = in;

    // scaling factor mainly used for borders,
    // based on vita resolution..
    scaling = std::min(renderer->getSize().x / 960, 1.0f);

    uiRomList = new GuiRomList(this, renderer->getSize());
    uiRomList->setLoadDelay(500);
    renderer->add(uiRomList);

    // build menus from options
    uiMenu = new GuiMenu(this);
    uiMenu->setVisibility(C2D_VISIBILITY_HIDDEN);
    renderer->add(uiMenu);

    uiEmu = new GuiEmu(this);
    uiEmu->setVisibility(C2D_VISIBILITY_HIDDEN);
    renderer->add(uiEmu);

    uiProgressBox = new GuiProgressBox(this);
    uiProgressBox->setVisibility(C2D_VISIBILITY_HIDDEN);
    renderer->add(uiProgressBox);

    uiMessageBox = new MessageBox(
            FloatRect(
                    renderer->getSize().x / 2,
                    renderer->getSize().y / 2,
                    renderer->getSize().x / 2,
                    renderer->getSize().y / 2),
            input, *skin->font, getFontSize());
    uiMessageBox->setOriginCenter();
    uiMessageBox->setFillColor(Color::GrayLight);
    uiMessageBox->setOutlineColor(Color::Orange);
    uiMessageBox->setOutlineThickness(2);
    renderer->add(uiMessageBox);
}

Gui::~Gui() {

    delete (uiMenu);
    delete (uiRomList);
}

void Gui::run() {

    int key = 0;
    updateInputMapping(false);

    while (true) {

        if (uiMenu->getVisibility() == C2D_VISIBILITY_VISIBLE) {
            key = uiMenu->update();
        } else if (uiEmu->getVisibility() == C2D_VISIBILITY_VISIBLE) {
            key = uiEmu->update();
        } else {
            key = uiRomList->update();
        }

        switch (key) {

            case UI_KEY_RUN_ROM:
                getInput()->clear(0);
                runRom(uiRomList->getRom());
                break;

            case UI_KEY_RESUME_ROM:
                getInput()->clear(0);
                uiEmu->resume();
                break;

            case UI_KEY_STOP_ROM:
                getInput()->clear(0);
                uiEmu->unload();
                uiRomList->setVisibility(C2D_VISIBILITY_VISIBLE);
                break;

            case UI_KEY_SHOW_MEMU_UI:
                getInput()->clear(0);
                uiMenu->loadMenu();
                uiMenu->setVisibility(C2D_VISIBILITY_VISIBLE);
                uiMenu->setLayer(1);
                break;

            case UI_KEY_SHOW_MEMU_ROM:
                getInput()->clear(0);
                getConfig()->load(uiRomList->getRom());
                uiMenu->loadMenu(true);
                uiMenu->setVisibility(C2D_VISIBILITY_VISIBLE);
                uiMenu->setLayer(1);
                break;

            case UI_KEY_FILTER_ROMS:
                uiRomList->updateRomList();
                break;

            case UI_KEY_SHOW_ROMLIST:
                getInput()->clear(0);
                uiMenu->setVisibility(C2D_VISIBILITY_HIDDEN);
                uiRomList->setVisibility(C2D_VISIBILITY_VISIBLE);
                break;

            case EV_QUIT:
                return;

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
            if (io->exist(path))
                break;
        }
    }

    if (!io->exist(path)) {
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

    // set per rom input scheme
    updateInputMapping(true);

    // load rom settings
    printf("RunRom: config->LoadRom(%s)\n", rom->zip);
    config->load(rom);

    printf("RunRom: uiEmu->load(%i)\n", nBurnDrvActive);
    uiEmu->load(nBurnDrvActive);
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

GuiRomList *Gui::getUiRomList() {
    return uiRomList;
}

GuiEmu *Gui::getUiEmu() {
    return uiEmu;
}

GuiProgressBox *Gui::getUiProgressBox() {
    return uiProgressBox;
}

MessageBox *Gui::getUiMessageBox() {
    return uiMessageBox;
}

Font *Gui::getFont() {
    return skin->font;
}

int Gui::getFontSize() {
    return config->getValue(Option::Index::SKIN_FONT_SIZE);
}

void Gui::updateInputMapping(bool isRomConfig) {

    if (isRomConfig) {
        input->setKeyboardMapping(config->getRomPlayerInputKeys(0));
        int dz = 2000 + config->getValue(Option::Index::JOY_DEADZONE, true) * 2000;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            input->setJoystickMapping(i, config->getRomPlayerInputButtons(i), dz);
            input->players[i].lx.id = config->getValue(Option::Index::JOY_AXIS_LX, true);
            input->players[i].ly.id = config->getValue(Option::Index::JOY_AXIS_LY, true);
            input->players[i].rx.id = config->getValue(Option::Index::JOY_AXIS_RX, true);
            input->players[i].ry.id = config->getValue(Option::Index::JOY_AXIS_RY, true);
        }
    } else {
        input->setKeyboardMapping(config->getGuiPlayerInputKeys(0));
        int dz = 2000 + config->getValue(Option::Index::JOY_DEADZONE) * 2000;
        for (int i = 0; i < PLAYER_COUNT; i++) {
            input->setJoystickMapping(i, config->getGuiPlayerInputButtons(i), dz);
            input->players[i].lx.id = config->getValue(Option::Index::JOY_AXIS_LX);
            input->players[i].ly.id = config->getValue(Option::Index::JOY_AXIS_LY);
            input->players[i].rx.id = config->getValue(Option::Index::JOY_AXIS_RX);
            input->players[i].ry.id = config->getValue(Option::Index::JOY_AXIS_RY);
        }
    }
}
