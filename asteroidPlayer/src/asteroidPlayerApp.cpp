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
    bool                serverWait[3], startScreen, gameOver;
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
    if(!serverWait[0] || !serverWait[1] || !serverWait[2]){
        
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
                        cout << " hit " ;
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
    gl::draw(bodies);
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
            scoreBoard.text("p" + to_string(i+1) + " - lives: " + to_string(lives[i]) +
                            "\n     score: " + to_string(scores[i]));
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
