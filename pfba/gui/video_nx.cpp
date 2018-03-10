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
        u32 *fb = (u32 *) gfxGetFramebuffer(&w, &h);

        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                fb[gfxGetFramebufferDisplayOffset((u32) x, (u32) y)] =
                        (u32) RGBA8_MAXALPHA(0, 0, 0);
            }
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gfxWaitForVsync();
    }
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
}

void NXVideo::draw(c2d::Transform &transform) {

    // dont draw with cross2d, we directly write to the framebuffer
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

    Vector2f screen = ui->getRenderer()->getSize();
    s32 vw = (s32) (screen.x / getScale().x);
    s32 vh = (s32) (screen.y / getScale().y);

    // rotation
    if (getRotation() == 0) {
        gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_V);
    } else if (getRotation() == 90) {
        gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_V
                              | NATIVE_WINDOW_TRANSFORM_ROT_90);
        vw = (s32) (screen.y / getScale().x);
        vh = (s32) (screen.x / getScale().y);
    } else if (getRotation() == 180) {
        gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_H);
    } else if (getRotation() == 270) {
        gfxConfigureTransform(NATIVE_WINDOW_TRANSFORM_FLIP_H
                              | NATIVE_WINDOW_TRANSFORM_ROT_90);
        vw = (s32) (screen.y / getScale().x);
        vh = (s32) (screen.x / getScale().y);
    }

    gfxConfigureResolution(vw, vh);
    u32 *fb_buf = (u32 *) gfxGetFramebuffer(&fb_w, &fb_h);

    int x, y;
    int w = (int) getSize().x;
    int h = (int) getSize().y;
    int cx = (fb_w - w) / 2;
    int cy = (fb_h - h) / 2;

    //printf("res:%ix%i | fb:%ix%i | tex:%ix%i | scale:%fx%f\n",
    //       vw, vh, fb_w, fb_h, (int) getSize().x, (int) getSize().y, getScale().x, getScale().y);

    float scanline_factor = 1;
    int effect = ui->getConfig()->getValue(Option::Index::ROM_SHADER, true);
    if (effect == 1) {
        // SCANLINE
        scanline_factor = 0.90f;
    } else if (effect == 2) {
        // SCANLINE+
        scanline_factor = 0.80f;
    }

    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {

            p = tex_buf[y * w + x];
            r = ((p & 0xf800) >> 11) << 3;
            g = ((p & 0x07e0) >> 5) << 2;
            b = (p & 0x001f) << 3;

            if (y % 2 == 0) {
                fb_buf[(u32) gfxGetFramebufferDisplayOffset((u32) x + cx, (u32) y + cy)] =
                        RGBA8_MAXALPHA(r, g, b);
            } else {
                fb_buf[(u32) gfxGetFramebufferDisplayOffset((u32) x + cx, (u32) y + cy)] =
                        RGBA8_MAXALPHA((u32) (r * scanline_factor),
                                       (u32) (g * scanline_factor),
                                       (u32) (b * scanline_factor));
            }
        }
    }
}

void NXVideo::updateScaling() {

    int rotated = 0;
    float rotation = 0;
    int rotation_cfg = ui->getConfig()->getValue(Option::Index::ROM_ROTATION, true);
    int scale_mode = ui->getConfig()->getValue(Option::Index::ROM_SCALING, true);
    int vertical = BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL;
    int flip = BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED;
    Vector2f screen = ui->getRenderer()->getSize();
    Vector2f scale_max;
    float sx = 1, sy = 1;

    gfxSetMode(GfxMode_TiledDouble);

    // clear fb before changing res/rot
    clear();

    if (vertical) {
        switch (rotation_cfg) {
            case 1: // ON
                rotation = flip ? 90 : 270;
                rotated = 1;
                break;
            case 2: // FLIP // TODO
            case 3: // CAB MODE
                rotation = flip ? 0 : 180;
                break;
            default: // OFF
                rotation = flip ? 180 : 0;
                break;
        }
    }

    if (rotated) {
        scale_max.x = screen.x / getSize().y;
        scale_max.y = screen.y / getSize().x;
    } else {
        scale_max.x = screen.x / getSize().x;
        scale_max.y = screen.y / getSize().y;
    }

    switch (scale_mode) {

        case 0: // 1x
            if (rotated) {
                // TODO
            }
            break;

        case 1: // 2x
        case 2: // 3x
            sx = sy = std::min(scale_max.x, (float) (scale_mode + 1));
            if (sy > scale_max.y) {
                sx = sy = std::min(scale_max.y, (float) (scale_mode + 1));
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

NXVideo::~NXVideo() {

    if (pixels) {
        free(pixels);
    }
}

#endif
