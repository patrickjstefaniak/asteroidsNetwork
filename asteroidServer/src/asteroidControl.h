//
//  asteroidControl.h
//  asteroidServer
//
//  Created by Patrick Stefaniak on 6/21/16.
//
//


#include "asteroid.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma once

class asteroidControl {
public:
    asteroidControl(){};
    asteroidControl(vec2 s[4][3]);
    
    void draw();
    vector<list<vec2>> update(vec2 s[4][3], list<vec2> bullets);
    void createAsteroids(int num);
    void clear();
    
    list<asteroid> mAsteroids;
    list<vec2>      shipPos;
    
};
