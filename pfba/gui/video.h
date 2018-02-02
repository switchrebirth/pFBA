//
// Created by cpasjuste on 25/11/16.
//

#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <cstring>
#include <c2d.h>

class Video : public c2d::C2DTexture {

public:

    Video(const c2d::Vector2f &size, c2d::Renderer *renderer);

    virtual ~Video();

    virtual void updateScaling();

private:

    c2d::Renderer *renderer = NULL;
    c2d::FloatRect scale;
    int rotation = 0;
};

#endif //_VIDEO_H_
