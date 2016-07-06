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
    
    vector<vec2>    constructBodies(vec2 sBody[]);
    list<vec2>      getBulletsPos();
    void            startGame();
    void            endGame();
    void            broadcast(osc::Message msg);
    void            drawInterface();
    void            listenerSetup();
    
    osc::SenderUdp      mSender0;
    osc::SenderUdp      mSender1;
    osc::SenderUdp      mSender2;
    osc::SenderUdp      mSender3;
    osc::ReceiverUdp    mReceiver0;
    osc::ReceiverUdp    mReceiver1;
    osc::ReceiverUdp    mReceiver2;
    osc::ReceiverUdp    mReceiver3;
    
    //first vec2 of pBody is center, second is direction
    vec2            pBody[4][2], pPoints[4][3];
    bool            pShoot[4], startScreen, gameOver, pActive[4], pResponse[4];
    int             pScore[4], pLives[4], menuDelay, numPlayers;
    list<bullet>    bullets;
    asteroidControl ac;
    TextBox         scoreBoard, title, spaceContinue;
};



asteroidServerApp::asteroidServerApp()
:mSender0(10005, "127.0.0.1", 20001)
,mReceiver0(10001)
,mSender1(10006, "127.0.0.1", 20002)
,mReceiver1(10002)
,mSender2(10007, "127.0.0.1", 20003)
,mReceiver2(10003)
,mSender3(10008, "127.0.0.1", 20004)
,mReceiver3(10004)
{
}




void asteroidServerApp::setup()
{
    setWindowSize(800, 600);
    startScreen = true;
    gameOver = false;
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
    for(int i = 0; i < 4; i ++){
        pShoot[i] = false;
        pActive[i] = false;
        pResponse[i] = false;
    }
    listenerSetup();
}




void asteroidServerApp::mouseDown( MouseEvent event )
{

}




void asteroidServerApp::update()
{
    if(startScreen){
 
        
        
    }else if(gameOver){
        if(menuDelay <= 0){
            gameOver = false;
            startScreen = true;
            for(int i = 0; i < 4 ; i++){
                pScore[i] = 0;
                pLives[i] = 3;
                pActive[i] = pResponse[i] = false;
            }
            ac.clear();
            bullets.clear();
            numPlayers = 0;
        }else{
            menuDelay --;
        }
        
        
    }else{
        for(int i = 0; i < 4; i++){
            if(pActive[i]){
                if(!pResponse[i]){
                    break;
                }
            }
        }
        //gameplay
        
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
        
        for(int i = 0; i < 4 ; i++){
            if(pActive[i] && pLives[i] > 0){
                break;
            }
            if(i == 3){
                endGame();
            }
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
        
        for(auto &p: pResponse){
            p = false;
        }
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


void asteroidServerApp::startGame()
{
    startScreen = false;
    int count = 0;
    for(int i = 0; i < 4 ; i++){
        if(pActive[i]){
            count ++;
        }else{
            pLives[i] = -1;
            pScore[i] = 0;
        }
    }
    numPlayers = count;
    
}

void asteroidServerApp::endGame()
{
    //send "/endgame/" message w a true bool
    //start timer
    gameOver = true;
    osc::Message end("/endgame/");
    end.append(true);
    broadcast(end);
    menuDelay = 500;
}

void asteroidServerApp::broadcast(osc::Message msg)
{
    mSender0.send(msg);
    mSender1.send(msg);
    mSender2.send(msg);
    mSender3.send(msg);
}

void asteroidServerApp::drawInterface()
{
    if(startScreen){
        title.text("a s t e r o i d s");
        gl::pushMatrices();
        gl::translate(vec2(0,getWindowHeight()/4 + 50));
        gl::draw(gl::Texture2d::create(title.render()));
        gl::translate(vec2(0,150));
        string activeplayers;
        for(int i = 0; i < 4; i++){
            if(pActive[i]){
                activeplayers = activeplayers + " " + to_string(i + 1);
            }
        }
        spaceContinue.text("players active: " + activeplayers);
        gl::draw(gl::Texture2d::create(spaceContinue.render()));
        gl::translate(vec2(0,100));
        spaceContinue.text("----waiting for players----");
        gl::draw(gl::Texture2d::create(spaceContinue.render()));
        gl::popMatrices();
    }else{
        gl::pushMatrices();
        gl::translate(vec2(-70, 0));
        int j = 0;
        for(int i = 0; i < numPlayers; i ++){
            while(!pActive[j]){
                j++;
            }
            scoreBoard.text("p" + to_string(j+1) + " - lives: " + to_string(pLives[j]) +
                            "\n     score: " + to_string(pScore[j]));
            gl::translate(vec2(getWindowWidth() * 1/(numPlayers+1), 0));
            gl::draw(gl::Texture2d::create(scoreBoard.render()));
        }
        gl::popMatrices();
        
        if(gameOver){
            title.text("g a m e   o v e r");
            gl::pushMatrices();
            gl::translate(vec2(0,getWindowHeight()/3 + 50));
            gl::draw(gl::Texture2d::create(title.render()));
            gl::popMatrices();
        }
    }
}

//trying to consolidate didnt work, sender/reciever objects didnt like it
void asteroidServerApp::listenerSetup()
{
    mSender0.bind();
    mReceiver0.setListener("/shipPos/",
                           [&](const osc::Message &msg){
                               pBody[0][0].x = msg[0].flt();
                               pBody[0][0].y = msg[1].flt();
                               pBody[0][1].x = msg[2].flt();
                               pBody[0][1].y = msg[3].flt();
                               pShoot[0] = msg[4].boolean();
                               pLives[0] = msg[5].int32();
                               pResponse[0] = true;
                           });
    mReceiver0.setListener("/start/",
                           [&](const osc::Message &msg){
                               //getting this message means player is present
                               ////send back message that it was recognized
                               pActive[0] = msg[0].boolean();
                               osc::Message starting("/startgame/");
                               starting.append(true);
                               ////send true if game is starting
                               //includes if the player is trying to start the game
                               if(msg[1].boolean()){
                                   startGame();
                               }
                               
                               if(!startScreen){
                                   starting.append(true);
                                   starting.append(int(numPlayers));
                                   starting.append(pActive[0]);
                                   starting.append(pActive[1]);
                                   starting.append(pActive[2]);
                                   starting.append(pActive[3]);
                               }else{
                                   starting.append(false);
                               }
                               mSender0.send(starting);
                           });
    mReceiver0.bind();
    mReceiver0.listen();
    
    mSender1.bind();
    mReceiver1.setListener("/shipPos/",
                           [&](const osc::Message &msg){
                               pBody[1][0].x = msg[0].flt();
                               pBody[1][0].y = msg[1].flt();
                               pBody[1][1].x = msg[2].flt();
                               pBody[1][1].y = msg[3].flt();
                               pShoot[1] = msg[4].boolean();
                               pLives[1] = msg[5].int32();
                               pResponse[1] = true;
                           });
    mReceiver1.setListener("/start/",
                           [&](const osc::Message &msg){
                               //getting this message means player is present
                               ////send back message that it was recognized
                               pActive[1] = msg[0].boolean();
                               osc::Message starting("/startgame/");
                               starting.append(true);
                               ////send true if game is starting
                               //includes if the player is trying to start the game
                               if(msg[1].boolean()){
                                   startGame();
                               }
                               
                               if(!startScreen){
                                   starting.append(true);
                                   starting.append(int(numPlayers));
                                   starting.append(pActive[0]);
                                   starting.append(pActive[1]);
                                   starting.append(pActive[2]);
                                   starting.append(pActive[3]);
                               }else{
                                   starting.append(false);
                               }
                               mSender1.send(starting);
                           });
    mReceiver1.bind();
    mReceiver1.listen();
    
    mSender2.bind();
    mReceiver2.setListener("/shipPos/",
                           [&](const osc::Message &msg){
                               pBody[2][0].x = msg[0].flt();
                               pBody[2][0].y = msg[1].flt();
                               pBody[2][1].x = msg[2].flt();
                               pBody[2][1].y = msg[3].flt();
                               pShoot[2] = msg[4].boolean();
                               pLives[2] = msg[5].int32();
                               pResponse[2] = true;
                           });
    mReceiver2.setListener("/start/",
                           [&](const osc::Message &msg){
                               //getting this message means player is present
                               ////send back message that it was recognized
                               pActive[2] = msg[0].boolean();
                               osc::Message starting("/startgame/");
                               starting.append(true);
                               ////send true if game is starting
                               //includes if the player is trying to start the game
                               if(msg[1].boolean()){
                                   startGame();
                               }
                               
                               if(!startScreen){
                                   starting.append(true);
                                   starting.append(int(numPlayers));
                                   starting.append(pActive[0]);
                                   starting.append(pActive[1]);
                                   starting.append(pActive[2]);
                                   starting.append(pActive[3]);
                               }else{
                                   starting.append(false);
                               }
                               mSender2.send(starting);
                           });
    mReceiver2.bind();
    mReceiver2.listen();
    
    mSender3.bind();
    mReceiver3.setListener("/shipPos/",
                           [&](const osc::Message &msg){
                               pBody[3][0].x = msg[0].flt();
                               pBody[3][0].y = msg[1].flt();
                               pBody[3][1].x = msg[2].flt();
                               pBody[3][1].y = msg[3].flt();
                               pShoot[3] = msg[4].boolean();
                               pLives[3] = msg[5].int32();
                               pResponse[3] = true;
                           });
    mReceiver3.setListener("/start/",
                           [&](const osc::Message &msg){
                               //getting this message means player is present
                               ////send back message that it was recognized
                               pActive[3] = msg[0].boolean();
                               osc::Message starting("/startgame/");
                               starting.append(true);
                               ////send true if game is starting
                               //includes if the player is trying to start the game
                               if(msg[1].boolean()){
                                   startGame();
                               }
                               
                               if(!startScreen){
                                   starting.append(true);
                                   starting.append(int(numPlayers));
                                   starting.append(pActive[0]);
                                   starting.append(pActive[1]);
                                   starting.append(pActive[2]);
                                   starting.append(pActive[3]);
                               }else{
                                   starting.append(false);
                               }
                               mSender3.send(starting);
                           });
    mReceiver3.bind();
    mReceiver3.listen();
}

CINDER_APP( asteroidServerApp, RendererGl )
