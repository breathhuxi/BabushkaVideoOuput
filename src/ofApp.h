#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxXmlSettings.h"

// listening port
#define PORT 7000

// max number of strings to display
#define NUM_MSG_STRINGS 20

// number of target sets
//#define NUM_TARGET_SETS 3

// number of targets
#define NUM_TARGETS 15

// 20 + 400 + 20 + 400 + 20 + 400 + 20 == 1280
// 20 + 240 + 20 + 240 + 20 + 240 + 20 == 800 // 20 + 760 + 20 = 800
#define MARGIN_LEFT 20
#define MARGIN_TOP 20
#define WIDTH 400
#define HEIGHT 240
#define FULL_HEIGHT 760

//the length of strings is dynamic, use class instead of struct
class Target{
public:
    int id;
    string name;
    bool isDetected;
    int detectedTimesCount;
    ofVec3f position;
    ofVec4f rotation;
    string leftMovieFileName;
    string middleMovieFileName;
    string rightMovieFileName;
    string soundFileName;
};

//struct here would work because the pointer and bool are static in size
struct VideoPlayer{
    ofVideoPlayer * video;
    bool playVideo;
};

struct SoundPlayer{
    ofSoundPlayer * sound;
    bool playSound;
};

class ofApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    ofTrueTypeFont font;
    ofxOscReceiver receiver;
    int currentMsgString;
    string msgStrings[NUM_MSG_STRINGS];
    float timers[NUM_MSG_STRINGS];
    ofxXmlSettings XML;
    
    // Set 0:
    //Target grandpa, grandma, dad, mom, child;
    // Set 1:
    //Target santa, snowman, penguin, reindeer, fox;
    // Set 2:
    //Target lion, tiger, pig, chimpanzee, bird;
    
    std::vector<Target*> targets;//doll markers
    //int dectectedTargetSet;
    std::vector<Target*> detectedTargets;
    
    int newDetectedMarkerId, playMovieTargetId, finishPlayingOneSet;
    VideoPlayer leftMovie, middleMovie, rightMovie;
    ofTrueTypeFont * titleFont, * subtitleFont;
    bool frameByframe;
    void reinitialize();

    void startVideo(VideoPlayer & videoPlayer, string videoName);
    void stopVideo(VideoPlayer & videoPlayer);
    
    SoundPlayer sound;
    void startSound(SoundPlayer & soundPlayer, string soundName);
    void stopSound(SoundPlayer & soundPlayer);
};

