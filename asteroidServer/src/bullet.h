//
//  bullet.h
//  asteroidServer
//
//  Created by Patrick Stefaniak on 6/20/16.
//
//

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma once

class bullet {
public:
    bullet();
    bullet(vec2 dir, vec2 pos, int sf);
    
    void update();
    void draw();
    void hit();
    
    vec2    pos, direction;
    int     lifeSpan, shotFrom;
    bool    isAlive;
};