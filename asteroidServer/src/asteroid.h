//
//  asteroid.h
//  asteroidServer
//
//  Created by Patrick Stefaniak on 6/21/16.
//
//

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma once

class asteroid {
public:
    asteroid(){}
    asteroid(vec2 center, bool big);
    
    void update();
    void draw();
    void updateBody();
    
    Shape2d body;
    vec2    center, direction;
    bool    isBig, isHit;
    
};