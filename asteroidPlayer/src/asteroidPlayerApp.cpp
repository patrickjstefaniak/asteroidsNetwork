#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Osc.h"
#include "ship.h"
#include "asteroid.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class asteroidPlayerApp : public App {
  public:
    asteroidPlayerApp();
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    void drawBody(vec2 serverS[]);
    void drawInterface();
    void keyDown(KeyEvent event) override;
    void keyUp(KeyEvent event) override;
    void switchSet(KeyEvent event, bool on);
    
    enum Control { RIGHT = 0, LEFT, UP, DOWN, SHOOT };
    bool    buttonsDown[5];
    
    osc::SenderUdp      mSender;
    osc::ReceiverUdp    mReciever;
    int                 player, bulletTimer, scores[4], lives[4], numPlayers, menuDelay;
    vec2                serverShips[4][2];
    vector<vec2>        serverBullets, hits;
    ship                mShip;
    bool                serverWait[4], startScreen, gameOver, serverStartReady,
                        wantToPlay, pActive[4], watching;
    TextBox             scoreBoard, title, spaceContinue;
    asteroid            mAsteroids;
};

asteroidPlayerApp::asteroidPlayerApp()
:mSender(atoi(getenv("socket")), "127.0.0.1", atoi(getenv("portOut")))
,mReciever(atoi(getenv("portIn")))
{
}

void asteroidPlayerApp::setup()
{
    setWindowSize(800, 600);
    startScreen = true;
    gameOver = watching = false;
    numPlayers = 4;
    menuDelay = 0;
    player = atoi(getenv("player"));
    spaceContinue = TextBox().font(Font("Courier" , 30)).size(vec2(getWindowWidth(), 50)).alignment(TextBox::CENTER);
    scoreBoard = TextBox().font(Font("Courier", 15)).size(vec2(getWindowWidth()/4, 50)).alignment(TextBox::LEFT);
    title = TextBox().font(Font("Courier", 70)).size(vec2(getWindowWidth(),100)).alignment(TextBox::CENTER);
    for(bool b: serverWait){
        b = true;
    }

    mReciever.setListener("/shipstate/",
                          [&](const osc::Message &msg){
                              //positions of ships
                              ////4 ships, center and direction
                              int i = 0;
                              for(int j = 0; j < 4; j++){
                                  serverShips[j][0].x = msg[i].flt();
                                  i++;
                                  serverShips[j][0].y = msg[i].flt();
                                  i++;
                                  serverShips[j][1].x = msg[i].flt();
                                  i++;
                                  serverShips[j][1].y = msg[i].flt();
                                  i++;
                              }
                              scores[0] = msg[16].int32();
                              scores[1] = msg[17].int32();
                              scores[2] = msg[18].int32();
                              scores[3] = msg[19].int32();
                              
                              lives[0] = msg[20].int32();
                              lives[1] = msg[21].int32();
                              lives[2] = msg[22].int32();
                              lives[3] = msg[23].int32();
                              
                              serverWait[0] = true;
                              //scores
                              //lives
                          });
    mReciever.setListener("/bullets/",
                          [&](const osc::Message &msg){
                              //first is number of bullets
                              int i = msg[0].int32();
                              serverBullets.clear();
                              for(int j = 1; j <= i * 2; j++){
                                  vec2 bul;
                                  bul.x = msg[j].flt();
                                  j++;
                                  bul.y = msg[j].flt();
                                  serverBullets.push_back(bul);
                              }
                              serverWait[1] = true;
                          });
    mReciever.setListener("/shipHits/",
                          [&](const osc::Message &msg){
                              int i = msg[0].int32();
                              hits.clear();
                              for(int j = 1; j <= i * 2; j++){
                                  vec2 ht;
                                  ht.x = msg[j].flt();
                                  j++;
                                  ht.y = msg[j].flt();
                                  hits.push_back(ht);
                              }
                              serverWait[2] = true;
                          });
    mReciever.setListener("/asteroids/",
                          [&](const osc::Message &msg){
                              int i = msg[0].int32();
                              mAsteroids.clear();
                              for(int j = 1; j <= i * 3; j++){
                                  vec2 as;
                                  as.x = msg[j].flt();
                                  j++;
                                  as.y = msg[j].flt();
                                  j++;
                                  mAsteroids.pushback(as, msg[j].boolean());
                              }
                              serverWait[3] = true;
                          });
    mReciever.setListener("/startgame/",
                          [&](const osc::Message &msg){
                              //receive confirmation that 'here' message was recieved
                              serverStartReady = msg[0].boolean();
                              //recieve start game message
                              if(msg[1].boolean()){
                              ////start game mode
                                  cout<< " starting ! ";
                                  numPlayers = msg[2].int32();
                                  pActive[0] = msg[3].boolean();
                                  pActive[1] = msg[4].boolean();
                                  pActive[2] = msg[5].boolean();
                                  pActive[3] = msg[6].boolean();
                                  if(wantToPlay){
                                      //start game
                                      startScreen = false;
                                  }else{
                                      //go into watch mode
                                      startScreen = false;
                                      watching = true;
                                  }
                              }
                          });
    mReciever.setListener("/endgame/",
                          [&](const osc::Message &msg){
                              //at any time server could send endgame message
                              if(msg[0].boolean()){
                                  //end game
                                  //start timer
                                  menuDelay = 500;
                                  gameOver = true;
                                  watching = false;
                                  mAsteroids.clear();
                                  cout << "game over ! ";
                              }
                          });
    mReciever.bind();
    mReciever.listen();
    mSender.bind();
    mShip = ship();
    for(bool &b: buttonsDown){
        b = false;
    }
    bulletTimer = 0;
}

void asteroidPlayerApp::mouseDown( MouseEvent event )
{
    
}

void asteroidPlayerApp::update()
{
    if(startScreen){
        //send 'here' message to server
        osc::Message here("/start/");
        //send true if wanting to play and false otherwise
        here.append(wantToPlay);
        //if ready to play and hitting shoot, send start game message
        if(serverStartReady && buttonsDown[SHOOT]){
            here.append(true);
        }else{
            here.append(false);
        }
        mSender.send(here);
        if(buttonsDown[UP] || buttonsDown[DOWN]){
            wantToPlay = !wantToPlay;
            buttonsDown[UP] = buttonsDown[DOWN] = false;
        }
        if(!wantToPlay){
            serverStartReady = false;
        }
        
        
        
    }else if(gameOver){
        //display gameover screen for x amount of time
        if(menuDelay <= 0){
            startScreen = true;
            gameOver = false;
            mShip = ship();
            for(bool &b: buttonsDown){
                b = false;
            }
            bulletTimer = 0;
            
        }else{
            menuDelay --;
        }
        
        
    ///game play
    }else{
        if(!serverWait[0] || !serverWait[1] || !serverWait[2] || !serverWait[3] || watching){
            
        }else{
            
            //see if ship is hit
            
            for(vec2 &h: hits){
                if(mShip.invincible <= 0){
                    for(vec2 p: mShip.body.getPoints()){
                        if(p == h){
                            if(mShip.lives == 0){
                                mShip.isActive = false;
                            }else{
                                mShip.die();
                            }
                        }
                    }
                }
            }
            
            mShip.update(buttonsDown);
            
            osc::Message msg("/shipPos/");
            msg.append(mShip.center.x);
            msg.append(mShip.center.y);
            msg.append(mShip.forward.x);
            msg.append(mShip.forward.y);
            if(!buttonsDown[SHOOT]){
                bulletTimer = 0;
                msg.append(false);
            }else{
                bulletTimer --;
                if(bulletTimer > 0){
                    msg.append(false);
                }else{
                    msg.append(true);
                    bulletTimer = 50;
                }
                
            }
            msg.append(int(mShip.lives));
            
            mSender.send(msg);
            
            for(bool b: serverWait){
                b = false;
            }
            serverWait[0] = serverWait[1] = serverWait[2] = serverWait[3] = false;
        }
    }
}

void asteroidPlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::color(1,1,1);
    for(int i = 0; i < 4; i++){
        if(i+1 != player){
            drawBody(serverShips[i]);
        }
    }
    mShip.draw();
    drawInterface();
    mAsteroids.draw();
}

void asteroidPlayerApp::drawBody(vec2 serverS[])
{
    Path2d bodies;
    bodies.moveTo(serverS[0] + (serverS[1] * 15.0f));
    vec2 perp = mat2(0,-1,1,0) * serverS[1];
    bodies.lineTo(serverS[0] + (perp * 5.0f));
    bodies.lineTo(serverS[0] - (perp * 5.0f));
    bodies.close();
    gl::color(.6,.6,.6);
    gl::draw(bodies);
    gl::color(1,1,1);
    for(vec2 &b: serverBullets){
        gl::drawSolidRect(Rectf(b - vec2(2), b + vec2(2)));
    }
}

void asteroidPlayerApp::drawInterface()
{
    gl::color(1,1,1);
    if(startScreen){
        title.text("a s t e r o i d s");
        gl::pushMatrices();
        gl::translate(vec2(0,getWindowHeight()/4 + 50));
        gl::draw(gl::Texture2d::create(title.render()));
        gl::translate(vec2(0,130));
        spaceContinue.text("player : " + to_string(player));
        gl::draw(gl::Texture2d::create(spaceContinue.render()));
        gl::translate(vec2(0,80));
        if(wantToPlay){
            spaceContinue.text("play next game? : y");
        }else{
            spaceContinue.text("play next game? : n");
        }
        gl::draw(gl::Texture2d::create(spaceContinue.render()));
        gl::translate(vec2(0,80));
        if(serverStartReady){
            spaceContinue.text("----press space to start----");
            gl::draw(gl::Texture2d::create(spaceContinue.render()));
        }
        gl::popMatrices();
    }else{
        gl::pushMatrices();
        gl::translate(vec2(-70, 0));
        int j = 0;
        for(int i = 0; i < numPlayers; i ++){
            while(!pActive[j]){
                j++;
            }
            scoreBoard.text("p" + to_string(j+1) + " - lives: " + to_string(lives[j]) +
                            "\n     score: " + to_string(scores[j]));
            if(j + 1 == player){
                scoreBoard.color(ColorA(1,1,1,1));
            }else{
                scoreBoard.color(ColorA(.5,.5,.5,1));
            }
            gl::translate(vec2(getWindowWidth() * 1/(numPlayers+1), 0));
            gl::draw(gl::Texture2d::create(scoreBoard.render()));
            j++;
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


void asteroidPlayerApp::keyDown(KeyEvent event)
{
    switchSet(event, true);
    
    
}

void asteroidPlayerApp::keyUp(KeyEvent event)
{
    switchSet(event, false);
}

//keys:  p1   p2  p3  p4
//RIGHT = right | d | h | l
//LEFT  = left  | a | f | j
//UP    = up    | w | t | i
//DOWN  = down  | s | g | k
//SHOOT = space | e | y | o

void asteroidPlayerApp::switchSet(KeyEvent event, bool on)
{
    switch (player){
        case 1:
            switch (event.getCode()){
                case KeyEvent::KEY_RIGHT:
                    buttonsDown[RIGHT] = on;
                    break;
                    
                case KeyEvent::KEY_LEFT:
                    buttonsDown[LEFT] = on;
                    break;
                    
                case KeyEvent::KEY_UP:
                    buttonsDown[UP] = on;
                    break;
                    
                case KeyEvent::KEY_DOWN:
                    buttonsDown[DOWN] = on;
                    break;
                    
                case KeyEvent::KEY_SPACE:
                    buttonsDown[SHOOT] = on;
                    break;
                    
                default:
                    break;
            }
            break;
            
            
        case 2:
            switch (event.getCode()){
                case KeyEvent::KEY_d:
                    buttonsDown[RIGHT] = on;
                    break;
                    
                case KeyEvent::KEY_a:
                    buttonsDown[LEFT] = on;
                    break;
                    
                case KeyEvent::KEY_w:
                    buttonsDown[UP] = on;
                    break;
                    
                case KeyEvent::KEY_s:
                    buttonsDown[DOWN] = on;
                    break;
                    
                case KeyEvent::KEY_e:
                    buttonsDown[SHOOT] = on;
                    break;
                    
                default:
                    break;
            }
            break;
            
            
        case 3:
            switch (event.getCode()){
                case KeyEvent::KEY_h:
                    buttonsDown[RIGHT] = on;
                    break;
                    
                case KeyEvent::KEY_f:
                    buttonsDown[LEFT] = on;
                    break;
                    
                case KeyEvent::KEY_t:
                    buttonsDown[UP] = on;
                    break;
                    
                case KeyEvent::KEY_g:
                    buttonsDown[DOWN] = on;
                    break;
                    
                case KeyEvent::KEY_y:
                    buttonsDown[SHOOT] = on;
                    break;
                    
                default:
                    break;
            }
            break;
            
            
        case 4:
            switch (event.getCode()){
                case KeyEvent::KEY_l:
                    buttonsDown[RIGHT] = on;
                    break;
                    
                case KeyEvent::KEY_j:
                    buttonsDown[LEFT] = on;
                    break;
                    
                case KeyEvent::KEY_i:
                    buttonsDown[UP] = on;
                    break;
                    
                case KeyEvent::KEY_k:
                    buttonsDown[DOWN] = on;
                    break;
                    
                case KeyEvent::KEY_o:
                    buttonsDown[SHOOT] = on;
                    break;
                    
                default:
                    break;
            }
            break;
            
            
        default:
            break;
    }
}

CINDER_APP( asteroidPlayerApp, RendererGl )
