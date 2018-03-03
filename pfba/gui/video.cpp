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

    this->ui = gui;

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

    int rotated = 0;
    float rotation = 0;
    int rotation_cfg = ui->getConfig()->getValue(Option::Index::ROM_ROTATION, true);
    int scale_mode = ui->getConfig()->getValue(Option::Index::ROM_SCALING, true);
    int vertical = BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL;
    int flip = BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED;
    Vector2f screen = ui->getRenderer()->getSize();
    Vector2f scale_max;
    float sx = 1, sy = 1;

#ifndef __PSP2__
    if (vertical) {
        switch (rotation_cfg) {
            case 1: // ON
                rotation = flip ? 90 : 270;
                rotated = 1;
                break;
            case 2: // CAB MODE
                rotation = flip ? 0 : 180;
                break;
            default: // OFF
                rotation = flip ? 180 : 0;
                break;
        }
    }
#else
    // TODO: force right to left orientation on psp2,
    // should add platform specific code
    if ((rotation_cfg == 0 || rotation_cfg == 3) && vertical) {
        if (!flip) {
            rotation = 180;
        }
    } else if (rotation_cfg == 2 && vertical) {
        if (flip) {
            rotation = 180;
        }
    } else {
        if (flip) {
            rotation = 90;
            rotated = 1;
        } else if (vertical) {
            rotation = -90;
            rotated = 1;
        } else {
            rotation = 0;
        }
    }
#endif

    if (rotated) {
        scale_max.x = screen.x / getSize().y;
        scale_max.y = screen.y / getSize().x;
    } else {
        scale_max.x = screen.x / getSize().x;
        scale_max.y = screen.y / getSize().y;
    }

    switch (scale_mode) {

        case 1: // 2x
            sx = sy = std::min(scale_max.x, 2.0f);
            if (sy > scale_max.y) {
                sx = sy = std::min(scale_max.y, 2.0f);
            }
            break;

        case 2: // 3x
            sx = sy = std::min(scale_max.x, 3.0f);
            if (sy > scale_max.y) {
                sx = sy = std::min(scale_max.y, 3.0f);
            }
            break;

        case 3: // fit
            sx = sy = scale_max.y;
            if (sx > scale_max.x) {
                sx = sy = scale_max.x;
            }
            break;

        case 4: // fit 4:3
            if (rotated) {
                sx = scale_max.y;
                float size_y = sx * getSize().x * 1.33f;
                sy = std::min(scale_max.x, size_y / getSize().y);
            } else {
                sy = scale_max.y;
                float size_x = sy * getSize().y * 1.33f;
                sx = std::min(scale_max.x, size_x / getSize().x);
            }
            break;

        case 5: // fullscreen
            sx = rotated ? scale_max.y : scale_max.x;
            sy = rotated ? scale_max.x : scale_max.y;
            break;

        default:
            break;
    }

    setOriginCenter();
    setPosition(screen.x / 2, screen.y / 2);
    setScale(sx, sy);
    setRotation(rotation);
}

Video::~Video() {
}

#endif // __NX__
