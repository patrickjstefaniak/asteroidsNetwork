#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Osc.h"
#include "bullet.h"

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
    bool            pShoot[4];
    list<bullet>    bullets;
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
    
    mReciever1.setListener("/shipPos/",
                          [&](const osc::Message &msg){
                              pBody[0][0].x = msg[0].flt();
                              pBody[0][0].y = msg[1].flt();
                              pBody[0][1].x = msg[2].flt();
                              pBody[0][1].y = msg[3].flt();
                              pShoot[0] = msg[4].boolean();
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

CINDER_APP( asteroidServerApp, RendererGl )
