#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Osc.h"
#include "ship.h"

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
    void keyDown(KeyEvent event) override;
    void keyUp(KeyEvent event) override;
    void switchSet(KeyEvent event, bool on);
    
    enum Control { RIGHT = 0, LEFT, UP, DOWN, SHOOT };
    bool    buttonsDown[5];
    
    osc::SenderUdp      mSender;
    osc::ReceiverUdp    mReciever;
    int                 player, bulletTimer;
    vec2                smPos;
    ship                mShip;
};

asteroidPlayerApp::asteroidPlayerApp()
:mSender(atoi(getenv("socket")), "127.0.0.1", atoi(getenv("portOut")))
,mReciever(atoi(getenv("portIn")))
{
}

void asteroidPlayerApp::setup()
{
    setWindowSize(800, 600);
    player = atoi(getenv("player"));
    mReciever.setListener("/mouseclick/",
                          [&](const osc::Message &msg){
                              smPos.x = msg[0].flt();
                              smPos.y = msg[1].flt();
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
    msg.append(buttonsDown[SHOOT]);
    
    mSender.send(msg);
}

void asteroidPlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::color(0,0,1);
    gl::drawSolidCircle(smPos, 10);
    mShip.draw();
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
