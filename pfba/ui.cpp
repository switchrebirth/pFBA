//
// Created by cpasjuste on 29/05/18.
//

#include "c2dui.h"
#include "ui.h"

#include "burn.h"

using namespace c2d;
using namespace c2dui;

PFBAGui::PFBAGui(Renderer *renderer, Io *io, Input *input, Audio *audio,
                 C2DUIConfig *config, C2DUISkin *skin)
        : C2DUIGuiMain(renderer, io, input, audio, config, skin) {

    printf("PFBAGui\n");
}

PFBAGui::~PFBAGui() {

    printf("~PFBAGui\n");
}

void PFBAGui::runRom(C2DUIRomList::Rom *rom) {

    if (!rom) {
        return;
    }

    nBurnDrvActive = rom->drv;
    if (nBurnDrvActive >= nBurnDrvCount) {
        printf("PFBAGui::runRom: driver not found\n");
        return;
    }

    // load rom settings
    printf("PFBAGui::runRom: config->load(%s)\n", rom->drv_name);
    getConfig()->load(rom);

    printf("PFBAGui::runRom: uiEmu->run(%i)\n", nBurnDrvActive);
    getUiEmu()->run(nBurnDrvActive);
}
