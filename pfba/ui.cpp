//
// Created by cpasjuste on 29/05/18.
//

#include "ui.h"
#include "c2dui.h"
#include "burn.h"

using namespace c2d;
using namespace c2dui;

PFBAGui::PFBAGui(c2d::Io *io, c2d::Renderer *rdr,
                 C2DUISkin *skin, c2dui::C2DUIConfig *cfg, c2d::Input *input, c2d::Audio *audio)
        : C2DUIGuiMain(io, rdr, skin, cfg, input, audio) {

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
    config->load(rom);

    printf("PFBAGui::runRom: uiEmu->run(%i)\n", nBurnDrvActive);
    uiEmu->run(nBurnDrvActive);
}
