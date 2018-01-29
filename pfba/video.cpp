//
// Created by cpasjuste on 01/12/16.
//

#include <burner.h>
#include <gui/config.h>
#include "video.h"

using namespace c2d;

extern Gui *gui;

static unsigned int myHighCol16(int r, int g, int b, int /* i */) {
    unsigned int t;
    t = (unsigned int) ((r << 8) & 0xf800); // rrrr r000 0000 0000
    t |= (g << 3) & 0x07e0; // 0000 0ggg ggg0 0000
    t |= (b >> 3) & 0x001f; // 0000 0000 000b bbbb
    return t;
}

Video::Video(const c2d::Vector2f &size, Renderer *renderer) : C2DTexture(size, C2D_TEXTURE_FMT_RGB565) {

    this->renderer = renderer;

    printf("game resolution: %ix%i\n", (int) getSize().x, (int) getSize().y);

    if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        printf("game orientation: vertical\n");
    }
    if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
        printf("game orientation: flipped\n");
    }

    nBurnBpp = 2;
    BurnHighCol = myHighCol16;
    BurnRecalcPal();
    lock(NULL, (void **) &pBurnDraw, &nBurnPitch);
    unlock();

    renderer->setShader(gui->getConfig()->getValue(Option::Index::ROM_SHADER, true));
    setFiltering(gui->getConfig()->getValue(Option::Index::ROM_FILTER, true));
    updateScaling();
}

void Video::updateScaling() {

    FloatRect window = renderer->getGlobalBounds();

    int scaling = gui->getConfig()->getValue(Option::Index::ROM_SCALING, true);
    rotation = 0;

    // TODO: force right to left orientation on psp2,
    // should add platform specific code

    if ((gui->getConfig()->getValue(Option::Index::ROM_ROTATION, true) == 0
         || gui->getConfig()->getValue(Option::Index::ROM_ROTATION, true) == 3)
        && BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        if (!(BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)) {
            rotation = 180;
        }
    } else if (gui->getConfig()->getValue(Option::Index::ROM_ROTATION, true) == 2
               && BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        if ((BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)) {
            rotation = 180;
        }
    } else {
        if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
            rotation = 90;
        } else if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
            rotation = -90;
        } else {
            rotation = 0;
        }
    }

    // TODO: force right to left orientation on psp2,
    // should add platform specific code

    scale.width = getSize().x;
    scale.height = getSize().y;

    switch (scaling) {

        case 1: // 2x
            scale.width = scale.width * 2;
            scale.height = scale.height * 2;
            break;

        case 2: // fit
            if (rotation == 0 || rotation == 180) {
                scale.height = window.height;
                scale.width = (int) (scale.width * (scale.height / getSize().y));
                if (scale.width > window.width) {
                    scale.width = window.width;
                    scale.height = (int) (scale.width * (getSize().y / getSize().x));
                }
            } else {
                scale.width = window.height;
                scale.height = (int) (scale.width * (getSize().y / getSize().x));
            }
            break;

        case 3: // fit 4:3
            if (rotation == 0 || rotation == 180) {
                scale.height = window.height;
                scale.width = (int) ((scale.height * 4.0) / 3.0);
                if (scale.width > window.width) {
                    scale.width = window.width;
                    scale.height = (int) ((scale.width * 3.0) / 4.0);
                }
            } else {
                scale.width = window.height;
                scale.height = (int) ((scale.width * 3.0) / 4.0);
            }
            break;

        case 4: // fullscreen
            if (rotation == 0 || rotation == 180) {
                scale.height = window.height;
                scale.width = window.width;
            } else {
                scale.height = window.width;
                scale.width = window.height;
            }
            break;

        default:
            break;
    }

    scale.left = (window.width - scale.width) / 2;
    scale.top = (window.height - scale.height) / 2;

    setPosition(scale.left, scale.top);
    setScale(scale.width / getSize().x, scale.height / getSize().y);
    setRotation(rotation);
}

Video::~Video() {

    pBurnDraw = NULL;
    renderer->setShader(0);
}
