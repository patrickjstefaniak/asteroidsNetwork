//
//  bullet.cpp
//  asteroidServer
//
//  Created by Patrick Stefaniak on 6/20/16.
//
//

#include "bullet.h"

bullet::bullet(){
}

bullet::bullet(vec2 dir, vec2 p, int sf){
    direction = dir;
    pos = p;
    lifeSpan = 100;
    isAlive = true;
    shotFrom = sf;
}


void bullet::draw(){
    gl::color(1,1,1);
    gl::drawSolidRect(Rectf(pos-vec2(2), pos+vec2(2)));
}

void bullet::update(){
    if(isAlive){
        pos += direction * 8.0f;
        lifeSpan -= 1;
        if(lifeSpan <= 0){
            isAlive = false;
        }
        if(pos.x < 0){
            pos.x = getWindowWidth();
        }else if(pos.x > getWindowWidth()){
            pos.x = 0;
        }
        if(pos.y < 0){
            pos.y = getWindowHeight();
        }else if(pos.y > getWindowHeight()){
            pos.y = 0;
        }
    }
}

void bullet::hit(){
    //move off screen until removed
    pos = vec2(100000);
    isAlive = false;
}