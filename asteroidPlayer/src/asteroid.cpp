//
//  asteroid.cpp
//  asteroidPlayer
//
//  Created by Patrick Stefaniak on 6/21/16.
//
//

#include "asteroid.h"


asteroid::asteroid(){
    positions.push_back(vec2(10000));
    sizes.push_back(false);
}

void asteroid::pushback(vec2 pos, bool isBig){
    positions.push_back(pos);
    sizes.push_back(isBig);
}

void asteroid::clear(){
    positions.clear();
    sizes.clear();
}

void asteroid::draw(){
    gl::color(1,1,1);
    auto i = positions.begin();
    for(bool big: sizes){
        if(big){
            Shape2d b;
            b.moveTo(*i + vec2(-40 , -24));
            b.lineTo(*i + vec2(-4 , -40));
            b.lineTo(*i + vec2(32  , -8));
            b.lineTo(*i + vec2(20  , 40));
            b.lineTo(*i + vec2(-8 , 32));
            b.lineTo(*i + vec2(-24 , 36));
            b.close();
            gl::draw(b);
        }else{
            Shape2d b;
            b.moveTo(*i + vec2(-4 , -8));
            b.lineTo(*i + vec2(4  , -10));
            b.lineTo(*i + vec2(8  , -2));
            b.lineTo(*i + vec2(4  , 2));
            b.lineTo(*i + vec2(6  , 8));
            b.lineTo(*i + vec2(-8 , 6));
            b.lineTo(*i + vec2(-10 , 1));
            b.close();
            gl::draw(b);
        }
        ++i;
    }
}