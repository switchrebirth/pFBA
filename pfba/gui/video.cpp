//
// Created by cpasjuste on 01/12/16.
//
#ifndef __NX__

#include <burner.h>
#include <gui/config.h>
#include "video.h"

using namespace c2d;

static unsigned int myHighCol16(int r, int g, int b, int /* i */) {
    unsigned int t;
    t = (unsigned int) ((r << 8) & 0xf800); // rrrr r000 0000 0000
    t |= (g << 3) & 0x07e0; // 0000 0ggg ggg0 0000
    t |= (b >> 3) & 0x001f; // 0000 0000 000b bbbb
    return t;
}

Video::Video(Gui *gui, const c2d::Vector2f &size) : C2DTexture(size, C2D_TEXTURE_FMT_RGB565) {

    this->gui = gui;

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

    setShader(gui->getConfig()->getValue(Option::Index::ROM_SHADER, true));
    setFiltering(gui->getConfig()->getValue(Option::Index::ROM_FILTER, true));
    updateScaling();
}

void Video::updateScaling() {


    int vertical = BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL;
    int flipped = BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED;
    int rotation_config = gui->getConfig()->getValue(Option::Index::ROM_ROTATION, true);
    int rotation_video = 0;

#ifdef __PSP2__
    // TODO: force right to left orientation on psp2,
    // should add platform specific code
    if ((gui->getConfig()->getValue(Option::Index::ROM_ROTATION, true) == 0
         || gui->getConfig()->getValue(Option::Index::ROM_ROTATION, true) == 3)
        && BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        if (!(BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)) {
            rotation_video = 180;
        }
    } else if (gui->getConfig()->getValue(Option::Index::ROM_ROTATION, true) == 2
               && BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        if ((BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)) {
            rotation_video = 180;
        }
    } else {
        if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
            rotation_video = 90;
        } else if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
            rotation_video = -90;
        } else {
            rotation_video = 0;
        }
    }
#else

    switch (rotation_config) {

        case 0: // OFF
            rotation_video = 0;
            break;

        case 1: // ON
            rotation_video = -90;
            break;

        case 2: // FLIP
            //rotation_video =
            break;

        case 3: // CAB MODE ?
            break;

        default:

            break;
    }

#endif

    int scaling = gui->getConfig()->getValue(Option::Index::ROM_SCALING, true);
    c2d::Vector2f scale = getSize();

    switch (scaling) {

        case 1: // 2x
            scale.x = scale.x * 2;
            scale.y = scale.y * 2;
            break;

        case 2: // fit
            if (rotation_video == 0 || rotation_video == 180) {
                scale.y = gui->getRenderer()->getSize().y;
                scale.x = (int) (scale.x * (scale.y / getSize().y));
                if (scale.x > gui->getRenderer()->getSize().x) {
                    scale.x = gui->getRenderer()->getSize().x;
                    scale.y = (int) (scale.x * (getSize().y / getSize().x));
                }
            } else {
                scale.x = gui->getRenderer()->getSize().y;
                scale.y = (int) (scale.x * (getSize().y / getSize().x));
            }
            break;

        case 3: // fit 4:3
            if (rotation_video == 0 || rotation_video == 180) {
                scale.y = gui->getRenderer()->getSize().y;
                scale.x = (int) ((scale.y * 4.0) / 3.0);
                if (scale.x > gui->getRenderer()->getSize().x) {
                    scale.x = gui->getRenderer()->getSize().x;
                    scale.y = (int) ((scale.x * 3.0) / 4.0);
                }
            } else {
                scale.x = gui->getRenderer()->getSize().y;
                scale.y = (int) ((scale.x * 3.0) / 4.0);
            }
            break;

        case 4: // fullscreen
            if (rotation_video == 0 || rotation_video == 180) {
                scale.y = gui->getRenderer()->getSize().y;
                scale.x = gui->getRenderer()->getSize().x;
            } else {
                scale.y = gui->getRenderer()->getSize().x;
                scale.x = gui->getRenderer()->getSize().y;
            }
            break;

        default:
            break;
    }

    setOriginCenter();
    setPosition(gui->getRenderer()->getSize().x / 2, gui->getRenderer()->getSize().y / 2);
    setScale(scale.x / getSize().x, scale.y / getSize().y);
    setRotation(rotation_video);
}

Video::~Video() {

    // TODO: free or not to free ?
    //pBurnDraw = NULL;
}

#endif // __NX__
