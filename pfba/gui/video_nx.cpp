//
// Created by cpasjuste on 01/12/16.
//

#ifdef __NX__

#include <burner.h>
#include <gui/config.h>
#include "video_nx.h"

using namespace c2d;

static unsigned int myHighCol16(int r, int g, int b, int /* i */) {
    unsigned int t;
    t = (unsigned int) ((r << 8) & 0xf800); // rrrr r000 0000 0000
    t |= (g << 3) & 0x07e0; // 0000 0ggg ggg0 0000
    t |= (b >> 3) & 0x001f; // 0000 0000 000b bbbb
    return t;
}

void NXVideo::clear() {

    for (int i = 0; i < 2; i++) {

        u32 w, h;
        u32 *dst = (u32 *) gfxGetFramebuffer(&w, &h);

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x += 4) {
                *((u128 *) &dst[gfxGetFramebufferDisplayOffset(x, y)]) = 0;
            }
        }

        //gfxFlushBuffers();
        gfxSwapBuffers();
        //gfxWaitForVsync();
    }

    gfxFlushBuffers();
    gfxWaitForVsync();
}

NXVideo::NXVideo(Gui *gui, const c2d::Vector2f &size) : Texture(size, C2D_TEXTURE_FMT_RGB565) {

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

    pixels = (unsigned char *) malloc((size_t) (size.x * size.y * bpp));

    updateScaling();

    clear();
}

int NXVideo::lock(c2d::FloatRect *rect, void **pix, int *p) {

    if (!rect) {
        *pix = pixels;
    } else {
        *pix = (void *) (pixels + (int) rect->top * pitch + (int) rect->left * bpp);
    }

    if (p) {
        *p = pitch;
    }

    return 0;
}

void NXVideo::unlock() {

    u32 fb_w, fb_h;
    unsigned short *tex_buf = (unsigned short *) pixels;
    unsigned int p, r, g, b;

    // give the ability to use slower (software)
    // scaling but with no filtering
    if (filtering == C2D_TEXTURE_FILTER_POINT && scale_mode < 3) {
        // 1x, 2x, 3x - software scaling
        gfxConfigureResolution(0, 0);
        u32 *fb_buf = (u32 *) gfxGetFramebuffer(&fb_w, &fb_h);
        unsigned subx, suby;
        int x, y, w, h, sf;
        int cx, cy;
        w = (int) getSize().x;
        h = (int) getSize().y;
        sf = scale_mode + 1;
        cx = (fb_w - (w * sf)) / 2;
        cy = (fb_h - (h * sf)) / 2;

        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {

                p = tex_buf[y * w + x];
                r = ((p & 0xf800) >> 11) << 3;
                g = ((p & 0x07e0) >> 5) << 2;
                b = (p & 0x001f) << 3;

                for (subx = 0; subx < sf; subx++) {
                    for (suby = 0; suby < sf; suby++) {
                        fb_buf[(u32) gfxGetFramebufferDisplayOffset(
                                (u32) ((x * sf) + subx + cx),
                                (u32) ((y * sf) + suby + cy))] =
                                RGBA8_MAXALPHA(r, g, b);
                    }
                }
            }
        }
    } else {

        Vector2f screen = ui->getRenderer()->getSize();
        s32 vw = (s32) (screen.x / getScale().x);
        s32 vh = (s32) (screen.y / getScale().y);
        gfxConfigureResolution(vw, vh);
        u32 *fb_buf = (u32 *) gfxGetFramebuffer(&fb_w, &fb_h);

        int x, y;
        int w = (int) getSize().x;
        int h = (int) getSize().y;
        int cx = (fb_w - w) / 2;
        int cy = (fb_h - h) / 2;

        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {

                p = tex_buf[y * w + x];
                r = ((p & 0xf800) >> 11) << 3;
                g = ((p & 0x07e0) >> 5) << 2;
                b = (p & 0x001f) << 3;

                fb_buf[(u32) gfxGetFramebufferDisplayOffset((u32) x + cx, (u32) y + cy)] =
                        RGBA8_MAXALPHA(r, g, b);
            }
        }
    }
}

void NXVideo::updateScaling() {

    int rotation_video = 0;
    int rotation_config = 0;
    int vertical = BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL;
    int flipped = BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED;

    // TODO: force right to left orientation on psp2,
    // should add platform specific code
    rotation_config = ui->getConfig()->getValue(Option::Index::ROM_ROTATION, true);

    switch (rotation_config) {

        case 1: // ON
            gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_ROT_90);
            rotation_video = 90;
            break;

        case 2: // FLIP
            rotation_video = 180;
            break;

        case 3: // CAB MODE ?
            break;

        default: // OFF
            gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_V);
            break;
    }

    scale_mode = ui->getConfig()->getValue(Option::Index::ROM_SCALING, true);
    c2d::Vector2f scaling = {1, 1};

    // on switch, the scaling factor is used
    // for virtual screen dimensions.
    // if filtering is point and 1x, 2x, 3x modes, the "texture" is software scaled
    // else if filtering is linear, the "texture" is hardware scaled
    switch (scale_mode) {

        case 1: // 2x
            scaling = {2, 2};
            break;

        case 2: // 3x
            scaling = {3, 3};
            break;

        case 3: // fit
            if (rotation_video == 0 || rotation_video == 180) {
                float f = std::min(
                        ui->getRenderer()->getSize().x / getSize().x,
                        ui->getRenderer()->getSize().y / getSize().y);
                scaling = {f, f};
            } else {
                // TODO
            }
            break;

        case 4: // fit 4:3
            if (rotation_video == 0 || rotation_video == 180) {
                float fx = std::min(
                        ui->getRenderer()->getSize().x / getSize().x,
                        ui->getRenderer()->getSize().y / getSize().y);
                scaling = {fx * (3.f / 4.f), fx};
            } else {
                // TODO
            }
            break;

        case 5: // fullscreen
            if (rotation_video == 0 || rotation_video == 180) {
                scaling.x = ui->getRenderer()->getSize().x / getSize().x;
                scaling.y = ui->getRenderer()->getSize().y / getSize().y;
            } else {
                // TODO
            }
            break;

        default:
            break;
    }

    setScale(scaling);
}

NXVideo::~NXVideo() {

    // TODO: to free or not to free pBurnDraw?
    //pBurnDraw = NULL;
    if (pixels) {
        free(pixels);
    }
}

#endif
