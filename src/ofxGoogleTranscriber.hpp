//
//  GFileTranscriber.hpp
//  ofx-cloud-speech
//
//  Created by Jack O'Shea on 6/13/17.
//
//

#ifndef ofxGoogleTranscriber_hpp
#define ofxGoogleTranscriber_hpp

#include <stdio.h>
#include "ofMain.h"


class ofxGoogleTranscriber {
    
    float minConfidence;
    
    float silenceThreshold;
    float maxSilenceDuration;
    unsigned maxSilentSamples;

    float maxChunkDuration;
    unsigned maxChunkSamples;

    unsigned sampleRate;
    string langCode;
    
    unsigned silentSampleCount;
    vector<int16_t> inBuffer;
    void flushInBuffer();
    void transcribe(vector<int16_t> pcms);

    atomic<unsigned> transcriberCount;
    atomic<bool> isDone;
    condition_variable noRemWorkers;
    void waitForTranscribers();
    
    mutex transcriptsMutex;
    stringstream transcripts;
    void appendTranscript(string transc);
    
    unsigned clientTranscrPos;
    
    atomic<bool> acceptInput;

public:
    void setup(unsigned _sampleRate=44100, float _minConfidence=0.85, string _langCode="en", float _maxChunkD=10.f);
    void setSilenceThreshold(float thresh, float duration=1.f);

    void addSoundBuffer(ofSoundBuffer& soundBuffer);
    void addSoundBufferSil(ofSoundBuffer& soundBuffer);
    
    void beginInput();
    void beginInput(string lang);
    void endOfInput();
    
    bool getIsDone();
    string getTranscript();
    string getNewTranscript();
    bool getNewTranscript(string* outString);
};

#endif /* GFileTranscriber_hpp */
