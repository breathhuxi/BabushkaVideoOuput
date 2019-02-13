#include "ofApp.h"

//---custom functions---

void ofApp::startVideo(VideoPlayer & videoPlayer, string videoName){
    std::cout << "video: " << videoPlayer.video << endl;
    if(videoPlayer.video){
        stopVideo(videoPlayer);
    }
    
    printf("start video\n");
    videoPlayer.playVideo = true;
    videoPlayer.video = new ofVideoPlayer();
    // Uncomment this to show movies with alpha channels
    // video->setPixelFormat(OF_PIXELS_RGBA);
    videoPlayer.video->load("movies/" + videoName);
    printf("video loaded\n");
    videoPlayer.video->setLoopState(OF_LOOP_NORMAL);
    videoPlayer.video->play();
    std::cout << "play " << videoName << " video: " << videoPlayer.playVideo << endl;
}

void ofApp::stopVideo(VideoPlayer & videoPlayer){
    printf("stop video\n");
    if(videoPlayer.video){
        videoPlayer.video->stop();
        videoPlayer.video->close();
        delete videoPlayer.video;
    }
    videoPlayer.video = 0;
    videoPlayer.playVideo = false;
}

void ofApp::startSound(SoundPlayer & soundPlayer, string soundName){
    std::cout << "sound: " << soundPlayer.sound << endl;
    if(soundPlayer.sound){
        stopSound(soundPlayer);
    }
    
    printf("start sound\n");
    soundPlayer.playSound = true;
    soundPlayer.sound = new ofSoundPlayer();
    soundPlayer.sound->load("sounds/" + soundName);
    printf("sound loaded\n");
    soundPlayer.sound->setLoop(true);
    soundPlayer.sound->play();
    std::cout << "play " << soundName << " sound: " << soundPlayer.playSound << endl;
}

void ofApp::stopSound(SoundPlayer & soundPlayer){
    printf("stop sound\n");
    if(soundPlayer.sound){
        soundPlayer.sound->stop();
        soundPlayer.sound->unload();//might have bugs: not sure if the sound resources are released
        delete soundPlayer.sound;
    }
    soundPlayer.sound = 0;
    soundPlayer.playSound = false;
}

void ofApp::reinitialize(){
    newDetectedMarkerId = -1;
    finishPlayingOneSet = 1;
    for (int i=0; i<NUM_TARGETS; i++){
        targets[i]->isDetected = false;
        targets[i]->detectedTimesCount = 0;
    }
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(255,255,255);
	ofSetVerticalSync(true);
    ofSetFrameRate(60);
	frameByframe = false;

    newDetectedMarkerId = -1;
    finishPlayingOneSet = -1;
    playMovieTargetId = -1;
    leftMovie.video = 0;
    leftMovie.playVideo = false;
    middleMovie.video = 0;
    middleMovie.playVideo = false;
    rightMovie.video = 0;
    rightMovie.playVideo = false;
    
    // set up OSC receiver, listening to PORT
    receiver.setup(PORT);
    
    // load font for title
    ofTrueTypeFont::setGlobalDpi(72);
    titleFont = new ofTrueTypeFont();
    titleFont->load("fonts/Iron.ttf", 96);
    subtitleFont = new ofTrueTypeFont();
    subtitleFont->load("fonts/Montserrat-Light.ttf", 36);
    
    // load xml settings to initialize doll markers
    if( XML.loadFile("BabushkaDollMarkerConfig.xml") ){
        std::cout << "BabushkaDollMarkerConfig.xml loaded!"<<endl;
    }else{
        std::cout << "unable to load BabushkaDollMarkerConfig.xml check data/ folder"<<endl;
    }
    
    XML.pushTag("root");//get deeper one level
    int numTags = XML.getNumTags("row");
    XML.popTag();//get back out one level
    
    if(XML.pushTag("root")){
        for (int i=0; i<numTags; i++) {
            if(XML.pushTag("row", i)){
                Target * t = new Target();//need new to allocate space to hold the variables
                t->id = i;
                t->name = XML.getValue("doll", "");
                t->leftMovieFileName = XML.getValue("left_movie", "");
                t->middleMovieFileName = XML.getValue("middle_movie", "");
                t->rightMovieFileName = XML.getValue("right_movie", "");
                t->soundFileName = XML.getValue("sound", "");
                t->isDetected = false;
                t->detectedTimesCount = 0;
                
                targets.push_back(t);
                
                XML.popTag();
            }
        }
        XML.popTag();
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    // hide old messages
    for (int i = 0; i < NUM_MSG_STRINGS; i++){
        if (timers[i] < ofGetElapsedTimef()){
            msgStrings[i] = "";
        }
    }
    
    // check for waiting messages
    while(receiver.hasWaitingMessages()){
        
        // get the next message
        ofxOscMessage m;
        receiver.getNextMessage(m);
        
        // check for image target messages
        Target detectedTarget;
        detectedTarget.name = m.getAddress();
        for (int i=0; i<NUM_TARGETS; i++) {
            if (detectedTarget.name == targets[i]->name){
                detectedTarget.id = targets[i]->id;
                break;
            }
        }
        
        targets[detectedTarget.id]->isDetected = m.getArgAsBool(0); // might have bugs: "true" => true (checked and work)
        if(targets[detectedTarget.id]->isDetected){
            targets[detectedTarget.id]->detectedTimesCount++;
            targets[detectedTarget.id]->position.set(m.getArgAsFloat(1), m.getArgAsFloat(2), m.getArgAsFloat(3));
            targets[detectedTarget.id]->rotation.set(m.getArgAsFloat(4), m.getArgAsFloat(5), m.getArgAsFloat(6), m.getArgAsFloat(7));
        }
        
        // unrecognized message
        string msgString;
        msgString = m.getAddress();
        msgString += ":";
        // TO-DO
        
        std::cout << "Target" << detectedTarget.id << " is detected: " << targets[detectedTarget.id]->isDetected << endl;
        std::cout << "Target" << detectedTarget.id << " is detected " << targets[detectedTarget.id]->detectedTimesCount << "times" << endl;
        std::cout << "Target" << detectedTarget.id << " position: " << targets[detectedTarget.id]->position << endl;
        std::cout << "Target" << detectedTarget.id << " rotation: " << targets[detectedTarget.id]->rotation << endl;
    }
    
    // check which corresponding movies and sound to play
    for (int i=14; NUM_TARGETS-i-1<=14; i--){
        if(finishPlayingOneSet < 1 && targets[i]->detectedTimesCount > 0){
            newDetectedMarkerId = i;
            break;
        }
    }
    
    std::cout << "newDetectedMarkerId: " << newDetectedMarkerId << endl;
    std::cout << "playMovieTargetId: " << playMovieTargetId << endl;
    std::cout << "finishPlayingOneSet: " << finishPlayingOneSet << endl;
    if(newDetectedMarkerId > playMovieTargetId || (finishPlayingOneSet == 1 && newDetectedMarkerId >= 0)){
        playMovieTargetId = newDetectedMarkerId;
        std::cout << "play video of target: " << playMovieTargetId << endl;
        finishPlayingOneSet = 0;
        
        // stop the previous videos and sound
        if(leftMovie.playVideo){
            stopVideo(leftMovie);
        }
        
        if(middleMovie.playVideo){
            stopVideo(middleMovie);
        }
        
        if(rightMovie.playVideo){
            stopVideo(rightMovie);
        }
        
        if(sound.playSound){
            stopSound(sound);
        }
        
        /*for (int i=0; i<NUM_TARGETS; i++) {
            std::cout << "Target " << targets[i]->id << ": " << targets[i]->name << endl;
            std::cout << "Left movie for " << targets[i]->id << ": " << targets[i]->leftMovieFileName << endl;
            std::cout << "Middle movie for " << targets[i]->id << ": " << targets[i]->middleMovieFileName << endl;
            std::cout << "Right movie for " << targets[i]->id << ": " << targets[i]->rightMovieFileName << endl;
        }*/
        
        // start the new videos and sound
        startVideo(leftMovie, targets[playMovieTargetId]->leftMovieFileName);
        startVideo(middleMovie, targets[playMovieTargetId]->middleMovieFileName);
        startVideo(rightMovie, targets[playMovieTargetId]->rightMovieFileName);
        startSound(sound, targets[playMovieTargetId]->soundFileName);
        
        if(finishPlayingOneSet == 0 && playMovieTargetId >= 0 && (playMovieTargetId+1)%5 == 0){
            reinitialize();
            std::cout << "newDetectedMarkerId: " << newDetectedMarkerId << endl;
        }
    }
    
    // update every update()
    if(playMovieTargetId >= 0){
        if(targets[playMovieTargetId]->detectedTimesCount >= 1){
            if(leftMovie.playVideo)
                leftMovie.video->update();
            if(middleMovie.playVideo)
                middleMovie.video->update();
            if(rightMovie.playVideo)
                rightMovie.video->update();
        }
        if(targets[playMovieTargetId]->detectedTimesCount == 0 && finishPlayingOneSet == 1){
            if(middleMovie.playVideo)
                middleMovie.video->update();
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    if(playMovieTargetId == -1){
        ofSetHexColor(0x000000);
        titleFont->drawString("Once upon a Culture", 200, 320);
        ofSetHexColor(0x6D6977);
        subtitleFont->drawString("Start the Journey", 220, 420);
    }else if((playMovieTargetId+1)%5 == 0){
        ofSetHexColor(0xFFFFFF);
        if(middleMovie.playVideo){
            middleMovie.video->draw(0, 20, 1280, 760);
        }
    }else{
        std::cout << "play left video: " << leftMovie.playVideo << endl;
        ofSetHexColor(0xFFFFFF);
        // left video
        if(leftMovie.playVideo){
            leftMovie.video->draw(MARGIN_LEFT,MARGIN_TOP,WIDTH,FULL_HEIGHT);
        }
        // middle video
        if(middleMovie.playVideo){
            middleMovie.video->draw(MARGIN_LEFT*2+WIDTH,MARGIN_TOP,WIDTH,FULL_HEIGHT);
        }
        // right video
        if(rightMovie.playVideo){
            rightMovie.video->draw(MARGIN_LEFT*3+WIDTH*2,MARGIN_TOP,WIDTH,FULL_HEIGHT);
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::keyPressed  (int key){
    //if new target is not recognized, arrow key -> to move on
    switch(key){
        case OF_KEY_RIGHT:
            if(newDetectedMarkerId < 14){
                newDetectedMarkerId++;
                targets[newDetectedMarkerId]->detectedTimesCount = 1;
            }
            break;
        case '0':
            newDetectedMarkerId = 0;
            targets[newDetectedMarkerId]->detectedTimesCount = 1;
            break;
        case '5':
            newDetectedMarkerId = 5;
            targets[newDetectedMarkerId]->detectedTimesCount = 1;
            break;
        case 'x':
            newDetectedMarkerId = 10;
            targets[newDetectedMarkerId]->detectedTimesCount = 1;
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	
}


//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
