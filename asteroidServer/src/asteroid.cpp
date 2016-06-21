//
//  asteroid.cpp
//  asteroidServer
//
//  Created by Patrick Stefaniak on 6/21/16.
//
//

#include "asteroid.h"


asteroid::asteroid(vec2 c, bool big){
    center = c;
    isBig = big;
    isHit = false;
    direction = normalize(vec2((rand() % 10) + 1, (rand() & 10) + 1));
}

void asteroid::update(){
    center += direction;
    
    //screen wrap around
    if(center.x < 0){
        center.x = getWindowWidth();
    }else if(center.x > getWindowWidth()){
        center.x = 0;
    }
    if(center.y < 0){
        center.y = getWindowHeight();
    }else if(center.y > getWindowHeight()){
        center.y = 0;
    }
    updateBody();
}

void asteroid::draw(){
    gl::color(1,1,1);
    gl::draw(body);
}

void asteroid::updateBody(){
    
    if(isBig){
        Shape2d b;
        b.moveTo(center + vec2(-40 , -24));
        b.lineTo(center + vec2(-4 , -40));
        b.lineTo(center + vec2(32  , -8));
        b.lineTo(center + vec2(20  , 40));
        b.lineTo(center + vec2(-8 , 32));
        b.lineTo(center + vec2(-24 , 36));
        b.close();
        body = b;
    }else{
        Shape2d b;
        b.moveTo(center + vec2(-4 , -8));
        b.lineTo(center + vec2(4  , -10));
        b.lineTo(center + vec2(8  , -2));
        b.lineTo(center + vec2(4  , 2));
        b.lineTo(center + vec2(6  , 8));
        b.lineTo(center + vec2(-8 , 6));
        b.lineTo(center + vec2(-10 , 1));
        b.close();
        body = b;
    }
    
}