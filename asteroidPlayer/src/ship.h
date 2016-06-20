//
//  ship.h
//  asteroidPlayer
//
//  Created by Patrick Stefaniak on 6/15/16.
//
//

using namespace ci;
using namespace ci::app;
using namespace std;

#pragma once

class ship {
public:
    ship();
    ship(vec2 pos);
    ship(bool active);
    
    void        draw();
    void        update(bool buttons[]);
    void        hit();
    void        die();
    void        constructBody();
    
    //forward is normalized vec of way ship is pointing
    vec2            velocity, forward , center ;
    Path2d          body;
    float           drag, turning, forwardMotion, size;
    int             lives, score, invincible, bulletDelay;
    bool            isActive;
};