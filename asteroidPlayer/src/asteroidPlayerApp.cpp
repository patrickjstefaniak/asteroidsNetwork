#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "Osc.h"

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
    
    osc::SenderUdp      mSender;
    osc::ReceiverUdp    mReciever;
    int                 player;
    vec2                smPos;
};

asteroidPlayerApp::asteroidPlayerApp()
:mSender(10000, "127.0.0.1", atoi(getenv("portOut")))
,mReciever(atoi(getenv("portIn")))
{
}

void asteroidPlayerApp::setup()
{
    player = atoi(getenv("player"));
    mReciever.setListener("/mouseclick/",
                          [&](const osc::Message &msg){
                              smPos.x = msg[0].int32();
                              smPos.y = msg[1].int32();
                          });
    mReciever.bind();
    mReciever.listen();
    mSender.bind();
}

void asteroidPlayerApp::mouseDown( MouseEvent event )
{
    osc::Message msg("/mouseclick/");
    msg.append(event.getX());
    msg.append(event.getY());
    
    mSender.send(msg);
}

void asteroidPlayerApp::update()
{
}

void asteroidPlayerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::color(0,0,1);
    gl::drawSolidCircle(smPos, 10);
}

CINDER_APP( asteroidPlayerApp, RendererGl )
