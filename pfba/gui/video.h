//
// Created by cpasjuste on 25/11/16.
//

#ifndef _VIDEO_H_
#define _VIDEO_H_

#ifdef __NX__

#include "video_nx.h"

#else
#include <cstring>
#include <c2d.h>

class Video : public c2d::C2DTexture {

public:

    Video(Gui *gui, const c2d::Vector2f &size);

    virtual ~Video();

    virtual void updateScaling();

private:

    Gui *gui = NULL;
};
#endif
#endif //_VIDEO_H_
