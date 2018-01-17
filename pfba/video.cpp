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

Video::Video(Renderer *renderer) {

    this->renderer = renderer;

    BurnDrvGetFullSize(&VideoBufferWidth, &VideoBufferHeight);
    printf("game resolution: %ix%i\n", VideoBufferWidth, VideoBufferHeight);

    if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL) {
        printf("game orientation: vertical\n");
    }
    if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED) {
        printf("game orientation: flipped\n");
    }

    if (screen == NULL) {
        screen = (Texture *) new C2DTexture(Vector2f(VideoBufferWidth, VideoBufferHeight));
    }

    nBurnBpp = 2;
    BurnHighCol = myHighCol16;
    BurnRecalcPal();
    screen->lock(NULL, (void **) &pBurnDraw, &nBurnPitch);
    screen->unlock();

    renderer->setShader(gui->getConfig()->getValue(Option::Index::ROM_SHADER, true));
    Filter(gui->getConfig()->getValue(Option::Index::ROM_FILTER, true));
    Scale();
}

void Video::Filter(int filter) {
    screen->setFiltering(filter);
    // SDL2 needs to regenerate a texture, so update burn buffer
    screen->lock(NULL, (void **) &pBurnDraw, &nBurnPitch);
    screen->unlock();
}

void Video::Scale() {

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

    //printf("rotation: %i\n", rotation);

    scale.width = VideoBufferWidth;
    scale.height = VideoBufferHeight;

    switch (scaling) {

        case 1: // 2x
            scale.width = scale.width * 2;
            scale.height = scale.height * 2;
            break;

        case 2: // fit
            if (rotation == 0 || rotation == 180) {
                scale.height = window.height;
                scale.width = (int) (scale.width * (scale.height / (float) VideoBufferHeight));
                if (scale.width > window.width) {
                    scale.width = window.width;
                    scale.height = (int) (scale.width * ((float) VideoBufferHeight / (float) VideoBufferWidth));
                }
            } else {
                scale.width = window.height;
                scale.height = (int) (scale.width * ((float) VideoBufferHeight / (float) VideoBufferWidth));
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

    //printf("scale: x=%i y=%i %ix%i\n", scale.x, scale.y, scale.width, scale.height);
    for (int i = 0; i < 3; i++) {
        Clear();
        Flip();
    }
}

void Video::Clear() {
    // TODO ?
    //renderer->clear();
}

void Video::Lock() {
    screen->lock(NULL, (void **) &pBurnDraw, &nBurnPitch);
}

void Video::Unlock() {
    screen->unlock();
}

// TODO ?
void Video::Render() {
    /*
    if (pBurnDraw != NULL) {
        screen->Draw(scale.x, scale.y, scale.width, scale.height, rotation);
    }
    */
}

void Video::Flip() {
    //renderer->Flip();
}

Video::~Video() {

    if (screen != NULL) {
        delete (screen);
        screen = NULL;
    }
    pBurnDraw = NULL;
    renderer->setShader(0);
}
