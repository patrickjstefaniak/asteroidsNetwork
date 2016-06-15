#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Osc.h"

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
    
    osc::SenderUdp      mSender1;
    osc::ReceiverUdp    mReciever1;
    osc::SenderUdp      mSender2;
    osc::ReceiverUdp    mReciever2;
    osc::SenderUdp      mSender3;
    osc::ReceiverUdp    mReciever3;
    osc::SenderUdp      mSender4;
    osc::ReceiverUdp    mReciever4;
    
    vec2 p1m;
    vec2 p2m;
    vec2 p3m;
    vec2 p4m;
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
    mReciever1.setListener("/mouseclick/",
                          [&](const osc::Message &msg){
                              p1m.x = msg[0].int32();
                              p1m.y = msg[1].int32();
                          });
    mReciever1.bind();
    mReciever1.listen();
    mSender1.bind();
    
    mReciever2.setListener("/mouseclick/",
                           [&](const osc::Message &msg){
                               p2m.x = msg[0].int32();
                               p2m.y = msg[1].int32();
                           });
    mReciever2.bind();
    mReciever2.listen();
    mSender2.bind();
    
    mReciever3.setListener("/mouseclick/",
                           [&](const osc::Message &msg){
                               p3m.x = msg[0].int32();
                               p3m.y = msg[1].int32();
                           });
    mReciever3.bind();
    mReciever3.listen();
    mSender3.bind();
    
    mReciever4.setListener("/mouseclick/",
                           [&](const osc::Message &msg){
                               p4m.x = msg[0].int32();
                               p4m.y = msg[1].int32();
                           });
    mReciever4.bind();
    mReciever4.listen();
    mSender4.bind();
}

void asteroidServerApp::mouseDown( MouseEvent event )
{
    osc::Message msg("/mouseclick/");
    msg.append(event.getX());
    msg.append(event.getY());
    
    mSender1.send(msg);
    mSender2.send(msg);
    mSender3.send(msg);
    mSender4.send(msg);
}

void asteroidServerApp::update()
{
    
}

void asteroidServerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
    gl::color(1,0,0);
    gl::drawSolidCircle(p1m, 10);
    gl::color(1,1,0);
    gl::drawSolidCircle(p2m, 10);
    gl::color(1,0,1);
    gl::drawSolidCircle(p3m, 10);
    gl::color(0,1,1);
    gl::drawSolidCircle(p4m, 10);
}

CINDER_APP( asteroidServerApp, RendererGl )
