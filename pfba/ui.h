//
// Created by cpasjuste on 29/05/18.
//

#ifndef PFBA_UI_H
#define PFBA_UI_H

#include <skeleton/io.h>
#include <skeleton/renderer.h>
#include <skeleton/input.h>

#include "c2dui.h"

class PFBAGui : public c2dui::C2DUIGuiMain {

    PFBAGui(c2d::Io *io, c2d::Renderer *rdr,
            c2dui::C2DUISkin *skin, c2dui::C2DUIConfig *cfg, c2d::Input *input, c2d::Audio *audio);

    void runRom(c2dui::C2DUIRomList::Rom *rom) override;
};


#endif //PFBA_UI_H
