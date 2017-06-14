#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    gCloudSpeech.setup();
    ofSoundStreamSetup(0, 2);
}

void ofApp::audioIn(ofSoundBuffer& soundBuffer) {
    gCloudSpeech.addSoundBufferSil(soundBuffer);
}

//--------------------------------------------------------------
void ofApp::update(){
    if (gCloudSpeech.getIsDone()) {
        transcript = gCloudSpeech.getTranscript();
        appState = kDone;
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    string str = "Waiting";
    switch (appState) {
        case kDefaultState:
            str = "Waiting";
            break;
        case kRecEnglish:
        case kRecDanish:
            str = "Recording";
            break;
        case kFinishing:
            str = "Processing";
            break;
        case kDone:
            str = transcript;
            break;
    }
    ofDrawBitmapString(str, 50, 50);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case 's':
            appState = kFinishing;
            gCloudSpeech.endOfInput();
            break;
        case 'e':
            appState = kRecEnglish;
            gCloudSpeech.beginInput("en");
            break;
        case 'd':
            appState = kRecDanish;
            gCloudSpeech.beginInput("da-DK");
            break;
        default: break;
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
