//
// Created by cpasjuste on 25/11/16.
//

#ifndef NX_VIDEO_H_
#define NX_VIDEO_H_

#include <cstring>
#include <c2d.h>

class NXVideo : public c2d::Texture {

public:

    NXVideo(Gui *gui, const c2d::Vector2f &size);

    ~NXVideo();

    void updateScaling();

    int lock(c2d::FloatRect *rect, void **pixels, int *pitch);

    void unlock();

    static void clear();

private:

    Gui *ui = NULL;
    unsigned char *pixels = NULL;
    int scale_mode = 0;
};

#endif //NX_VIDEO_H_
