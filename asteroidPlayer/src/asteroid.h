//
//  asteroid.h
//  asteroidPlayer
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
    asteroid();
    
    void draw();
    void pushback(vec2 pos, bool isBig);
    void clear();
    
    vector<vec2> positions;
    vector<bool> sizes;
};