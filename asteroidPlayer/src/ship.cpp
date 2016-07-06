//
//  ship.cpp
//  asteroidPlayer
//
//  Created by Patrick Stefaniak on 6/15/16.
//
//

#include "ship.h"

ship::ship()
{
    forward = vec2(0,-1);
    drag = 1.009;
    lives = 3;
    center = getWindowCenter();
    velocity = vec2(0);
    turning = 0;
    constructBody();
    forwardMotion = 0;
    size = 15;
    score = 0;
    invincible = 100;
    isActive = true;
    bulletDelay = 0;
}

ship::ship(vec2 pos)
{
    
}

ship::ship(bool active){
    isActive = active;
}

void ship::draw()
{
    gl::color(1,1,1);
    if(invincible > 0){
        gl::color(.5,.5,.5);
    }
    if(isActive){
        gl::draw(body);
    }
}


void ship::update(bool buttons[])
{
    if(isActive){
        
        //add forces based on buttons pressed
        if(buttons[0]){
            turning += 0.05f;
        }
        if(buttons[1]){
            turning -= 0.05f;
        }
        if(buttons[2]){
            forwardMotion += 0.3f;
        }
        if(buttons[3]){
            forwardMotion -= 0.3f;
        }
        
        if(invincible >= 0){
            invincible --;
        }
        
        
        //turn ship
        forward = forward * mat2(cos(turning), -sin(turning), sin(turning), cos(turning));
        forward = normalize(forward);
        turning = 0;
        
        
        //adjust acceleration
        vec2 newVel = forwardMotion * forward;
        forwardMotion = 0;
        
        
        //limit max speed
        if(length(velocity) > 3){
            vec2 velDir = normalize(velocity);
            velocity = velDir * 3.0f;
        }
        
        
        //apply current velocity and drag
        velocity += newVel;
        if(length(velocity) > 0){
            velocity = velocity / drag;
        }
        
        //adjust position
        center += velocity;
        
        
        //check to see if off screen, wrap to other side
        if(center.x < 0){
            center.x = getWindowWidth() + (size / 2);
        }else if(center.x > getWindowWidth() + (size / 2)){
            center.x = 0;
        }
        if(center.y < 0){
            center.y = getWindowHeight() + (size / 2);
        }else if(center.y > getWindowHeight() + (size / 2)){
            center.y = 0;
        }
        
        constructBody();
        
    }else{
        center = vec2(10000);
    }
}



void ship::hit(){
    score += 100;
}

void ship::die()
{
    //period after restarting so that ship doesnt start on
    //top of asteroid or get immediately hit by one
    
    center = getWindowCenter();
    lives -= 1;
    velocity = vec2(0);
    forward = vec2(0,-1);
    invincible = 150;
    if(lives <= 0){
        //move offscreen?
        //wait until new game, other ships could still be playing
        isActive = false;
    }
    
}

void ship::constructBody()
{
    //build ship shape based on center position and forward direction
    body.clear();
    body.moveTo(center + (forward * size));
    vec2 perp = mat2(0, -1, 1, 0) * forward;
    body.lineTo(center + (perp * size / 3.0f));
    body.lineTo(center - (perp * size / 3.0f));
    body.close();
}