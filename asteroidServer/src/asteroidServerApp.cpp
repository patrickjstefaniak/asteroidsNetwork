#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Osc.h"
#include "bullet.h"
#include "asteroidControl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class asteroidServerApp : public App {
  public:
    asteroidServerApp();
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    vector<vec2> constructBodies(vec2 sBody[]);
    list<vec2> getBulletsPos();
    void broadcast(osc::Message msg);
    void drawInterface();
    void startingCommunication();
    
    
    osc::SenderUdp      mSender1;
    osc::ReceiverUdp    mReciever1;
    osc::SenderUdp      mSender2;
    osc::ReceiverUdp    mReciever2;
    osc::SenderUdp      mSender3;
    osc::ReceiverUdp    mReciever3;
    osc::SenderUdp      mSender4;
    osc::ReceiverUdp    mReciever4;
    
    //first vec2 of pBody is center, second is direction
    vec2            pBody[4][2], pPoints[4][3];
    bool            pShoot[4], startScreen, gameOver;
    int             pScore[4], pLives[4], menuDelay, numPlayers;
    list<bullet>    bullets;
    asteroidControl ac;
    TextBox         scoreBoard, title, spaceContinue;
};



asteroidServerApp::asteroidServerApp()
:mSender1(10005, "127.0.0.1", 20001)
,mReciever1(10001)
,mSender2(10006, "127.0.0.1", 20002)
,mReciever2(10002)
,mSender3(10007, "127.0.0.1", 20003)
,mReciever3(10003)
,mSender4(10008, "127.0.0.1", 20004)
,mReciever4(10004)
{
}




void asteroidServerApp::setup()
{
    setWindowSize(800, 600);
    startScreen = gameOver = false;
    ac = asteroidControl(pPoints);
    spaceContinue = TextBox().font(Font("Courier" , 30)).size(vec2(getWindowWidth(), 50)).alignment(TextBox::CENTER);
    scoreBoard = TextBox().font(Font("Courier", 15)).size(vec2(getWindowWidth()/4, 50)).alignment(TextBox::LEFT);
    title = TextBox().font(Font("Courier", 70)).size(vec2(getWindowWidth(),100)).alignment(TextBox::CENTER);
    numPlayers = 4;
    for(int &i: pScore){
        i = 0;
    }
    for(int &i: pLives){
        i = 3;
    }
    
    //set up listeners
    mReciever1.setListener("/shipPos/",
                          [&](const osc::Message &msg){
                              pBody[0][0].x = msg[0].flt();
                              pBody[0][0].y = msg[1].flt();
                              pBody[0][1].x = msg[2].flt();
                              pBody[0][1].y = msg[3].flt();
                              pShoot[0] = msg[4].boolean();
                              pLives[0] = msg[5].int32();
                          });
    mReciever1.bind();
    mReciever1.listen();
    mSender1.bind();
    
    mReciever2.setListener("/shipPos/",
                           [&](const osc::Message &msg){
                               pBody[1][0].x = msg[0].flt();
                               pBody[1][0].y = msg[1].flt();
                               pBody[1][1].x = msg[2].flt();
                               pBody[1][1].y = msg[3].flt();
                               pShoot[1] = msg[4].boolean();
                               pLives[1] = msg[5].int32();
                           });
    mReciever2.bind();
    mReciever2.listen();
    mSender2.bind();
    
    mReciever3.setListener("/shipPos/",
                           [&](const osc::Message &msg){
                               pBody[2][0].x = msg[0].flt();
                               pBody[2][0].y = msg[1].flt();
                               pBody[2][1].x = msg[2].flt();
                               pBody[2][1].y = msg[3].flt();
                               pShoot[2] = msg[4].boolean();
                               pLives[2] = msg[5].int32();
                           });
    mReciever3.bind();
    mReciever3.listen();
    mSender3.bind();
    
    mReciever4.setListener("/shipPos/",
                           [&](const osc::Message &msg){
                               pBody[3][0].x = msg[0].flt();
                               pBody[3][0].y = msg[1].flt();
                               pBody[3][1].x = msg[2].flt();
                               pBody[3][1].y = msg[3].flt();
                               pShoot[3] = msg[4].boolean();
                               pLives[3] = msg[5].int32();
                           });
    mReciever4.bind();
    mReciever4.listen();
    mSender4.bind();
    for(int i = 0; i < 4; i ++){
        pShoot[i] = false;
    }
}




void asteroidServerApp::mouseDown( MouseEvent event )
{
    /*osc::Message msg("/mouseclick/");
    msg.append(event.getX());
    msg.append(event.getY());
    
    mSender1.send(msg);
    mSender2.send(msg);
    mSender3.send(msg);
    mSender4.send(msg); */
}




void asteroidServerApp::update()
{
    if(startScreen){
        //listen to see what players are connected
        ////send message to them to tell them they are connected?
        //if one of them presses shoot
        ////count number of players
        ////create asteroids
        //what to do if new players try to connect?
        ////not allow
        ////send back message? or just ignore
    }else if(gameOver){
        //see if button pressed to move to start menu
        ////erase scores, lives back to 3
        ////reset ship positions (offscreen?)
        ////clear asteroids
        ////menudelay to 100
        ////numplayers?
    }else{
        //update ship positions
        for(int i = 0; i < 4; i++){
            auto nBody = constructBodies(pBody[i]);
            auto iter = nBody.begin();
            pPoints[i][0] = *iter;
            ++iter;
            pPoints[i][1] = *iter;
            ++iter;
            pPoints[i][2] = *iter;
        }
        
        //see if there are new bullets
        for(int i = 0; i < 4; i++){
            if(pShoot[i]){
                bullets.push_back(bullet(pBody[i][1], pBody[i][0], i));
            }
        }
        
        //see if any asteroids were hit
        //ac.update returns list of asteroids hit by bullets
        //and list of ships hit by asteroids respectively
        vector<list<vec2>> hits = ac.update(pPoints, getBulletsPos());
        for(vec2 &h : hits.front()){
            for(bullet &b: bullets){
                if(b.pos == h){
                    b.hit();
                    pScore[b.shotFrom] += 100;
                }
            }
        }
        
        //update bullets, remove ones that are expired
        for(list<bullet>::iterator b = bullets.begin(); b!=bullets.end();){
            b->update();
            if(!b->isAlive){
                auto c = b;
                ++b;
                bullets.erase(c);
            }else{
                ++b;
            }
        }
        
        //see if game over
        
        if(pLives[0] == 0 && pLives[1] == 0 && pLives[2] == 0 && pLives[3] == 0){
            gameOver = true;
            menuDelay = 100;
        }
        
        //send report to other players
        osc::Message msg("/shipstate/");
        //positions of ships
        for(int j = 0; j < 4; j++){
            msg.append(pBody[j][0].x);
            msg.append(pBody[j][0].y);
            msg.append(pBody[j][1].x);
            msg.append(pBody[j][1].y);
        }
        msg.append(pScore[0]);
        msg.append(pScore[1]);
        msg.append(pScore[2]);
        msg.append(pScore[3]);
        msg.append(pLives[0]);
        msg.append(pLives[1]);
        msg.append(pLives[2]);
        msg.append(pLives[3]);
        broadcast(msg);
        //scores
        //lives
        //pos of hit ships
        osc::Message msgh("/shipHits/");
        msgh.append(int(hits.back().size()));
        for(vec2 &h: hits.back()){
            msgh.append(float(h.x));
            msgh.append(float(h.y));
        }
        broadcast(msgh);
        
        ///separate messages?
        //positions of asteroids
        //positions of bullets
        osc::Message msgb("/bullets/");
        msgb.append(int(bullets.size()));
        for(bullet &b: bullets){
            msgb.append(b.pos.x);
            msgb.append(b.pos.y);
        }
        broadcast(msgb);
        
        osc::Message msga("/asteroids/");
        msga.append(int(ac.mAsteroids.size()));
        for(asteroid &a: ac.mAsteroids){
            msga.append(float(a.center.x));
            msga.append(float(a.center.y));
            msga.append(a.isBig);
        }
        broadcast(msga);
        
        
    }
}




void asteroidServerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    Path2d ship;
    for(int i = 0; i < 4; i++){
        ship.clear();
        ship.moveTo(pPoints[i][0]);
        ship.lineTo(pPoints[i][1]);
        ship.lineTo(pPoints[i][2]);
        ship.close();
        gl::draw(ship);
    }
    for(bullet &b: bullets){
        b.draw();
    }
    ac.draw();
    drawInterface();
}




vector<vec2> asteroidServerApp::constructBodies(vec2 shipInfo[])
{
    //build ship shape based on center position and forward direction
    vector<vec2> newBody;
    newBody.push_back(shipInfo[0] + (shipInfo[1] * 15.0f));
    vec2 perp = mat2(0, -1, 1, 0) * shipInfo[1];
    newBody.push_back(shipInfo[0] + (perp * 5.0f));
    newBody.push_back(shipInfo[0] - (perp * 5.0f));
    return newBody;
}




list<vec2> asteroidServerApp::getBulletsPos()
{
    list<vec2> buls;
    for(bullet &b : bullets){
        buls.push_back(b.pos);
    }
    return buls;
}




void asteroidServerApp::broadcast(osc::Message msg)
{
    mSender1.send(msg);
    mSender2.send(msg);
    mSender3.send(msg);
    mSender4.send(msg);
}

void asteroidServerApp::drawInterface()
{
    if(startScreen){
        title.text("a s t e r o i d s");
        gl::pushMatrices();
        gl::translate(vec2(0,getWindowHeight()/4 + 50));
        gl::draw(gl::Texture2d::create(title.render()));
        gl::translate(vec2(0,150));
        spaceContinue.text("players: " + to_string(numPlayers));
        gl::draw(gl::Texture2d::create(spaceContinue.render()));
        gl::translate(vec2(0,100));
        if(menuDelay <= 0){
            spaceContinue.text("----press space to start----");
            gl::draw(gl::Texture2d::create(spaceContinue.render()));
        }
        gl::popMatrices();
    }else{
        gl::pushMatrices();
        gl::translate(vec2(-70, 0));
        for(int i = 0; i < numPlayers; i ++){
            scoreBoard.text("p" + to_string(i+1) + " - lives: " + to_string(pLives[i]) +
                            "\n     score: " + to_string(pScore[i]));
            gl::translate(vec2(getWindowWidth() * 1/(numPlayers+1), 0));
            gl::draw(gl::Texture2d::create(scoreBoard.render()));
        }
        gl::popMatrices();
        
        if(gameOver){
            title.text("g a m e   o v e r");
            gl::pushMatrices();
            gl::translate(vec2(0,getWindowHeight()/3 + 50));
            gl::draw(gl::Texture2d::create(title.render()));
            gl::translate(vec2(0,100));
            if(menuDelay <= 0){
                spaceContinue.text("----press space to continue----");
                gl::draw(gl::Texture2d::create(spaceContinue.render()));
            }
            gl::popMatrices();
        }
    }
}

void asteroidServerApp::startingCommunication()
{
    
}

CINDER_APP( asteroidServerApp, RendererGl )
