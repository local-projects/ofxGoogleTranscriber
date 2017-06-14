//
//  ofxGoogleTranscriber.cpp
//  ofx-cloud-speech
//
//  Created by Jack O'Shea on 6/13/17.
//
//

#include "ofxGoogleTranscriber.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <grpc++/grpc++.h>
#include <unistd.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#include "google/cloud/speech/v1/cloud_speech.grpc.pb.h"
#include "google/longrunning/operations.grpc.pb.h"

using namespace google::cloud::speech::v1;
using google::cloud::speech::v1::RecognitionConfig;
using google::cloud::speech::v1::RecognitionAudio;
using google::cloud::speech::v1::Speech;
using google::cloud::speech::v1::RecognizeRequest;
using google::cloud::speech::v1::RecognizeResponse;


static int16_t getPCM(float f) {
    return f * (1 << 15);
}

void ofxGoogleTranscriber::appendTranscript(string transc) {
    lock_guard<mutex> guard(transcriptsMutex);
    transcripts << ' ' << transc;
}

void ofxGoogleTranscriber::transcribe(vector<int16_t> outBuffer) {
 
    ofLogNotice("ofxGoogleTranscriber") << "Thread " << this_thread::get_id()
        << " beginning transcription of " << outBuffer.size() << " samples";
   
    auto creds = grpc::GoogleDefaultCredentials();
    auto channel = grpc::CreateChannel("speech.googleapis.com", creds);
    std::unique_ptr<Speech::Stub> speech(Speech::NewStub(channel));

    RecognizeRequest request;
    RecognitionConfig* config = request.mutable_config();
    config->set_encoding(RecognitionConfig::LINEAR16);
    config->set_sample_rate_hertz(sampleRate);
    config->set_language_code(langCode);
    
    request.mutable_audio()->set_content(&outBuffer[0], outBuffer.size() * 2);

    grpc::ClientContext context;
    RecognizeResponse response;
    grpc::Status rpc_status = speech->Recognize(&context, request, &response);
    
    if (!rpc_status.ok()) {
        ofLogError("ofxGoogleTranscriber") << "Code " << rpc_status.error_code()
                                    << rpc_status.error_message() << endl;
    }

    for (int r = 0; r < response.results_size(); ++r) {
        auto result = response.results(r);
        for (int a = 0; a < result.alternatives_size(); ++a) {
        
            auto alternative = result.alternatives(a);
            float confidence = alternative.confidence();
            if (confidence >= minConfidence)
                appendTranscript(alternative.transcript());
            ofLogNotice("ofxGoogleTranscriber") << confidence << "\t" << alternative.transcript();
        }
    }
    
    transcriberCount--;
    ofLogNotice("ofxGoogleTranscriber") << "Thread " << this_thread::get_id() << " finished\n";

    noRemWorkers.notify_one();
    
}

void ofxGoogleTranscriber::flushInBuffer() {
    if (silentSampleCount != inBuffer.size()) {
        thread transcriber(&ofxGoogleTranscriber::transcribe, this, inBuffer);
        transcriber.detach();
        transcriberCount++;
    }
    inBuffer.clear();
    silentSampleCount = 0;
}

void ofxGoogleTranscriber::waitForTranscribers() {
    mutex finishMutex;
    unique_lock<mutex> finishLock(finishMutex);
    noRemWorkers.wait(finishLock, [this] { return !transcriberCount; } );
    isDone = true;
    ofLogNotice("ofxGoogleTranscriber") << "Finished transcription\t" << transcripts.str() << endl;
}

//-----------------------------------------------------------------------------
// MARK: -                  Public Methods
//-----------------------------------------------------------------------------

/*****************************************************************************/
void ofxGoogleTranscriber::setup(unsigned _sampleRate, float _minConfidence, string _langCode, float _maxChunkD) {
    langCode = _langCode;
    sampleRate = _sampleRate;
    minConfidence = _minConfidence;
    maxChunkDuration = _maxChunkD;
    maxChunkSamples = sampleRate * maxChunkDuration;
    
    silenceThreshold = 0.001;
    maxSilenceDuration = 1.f;
    maxSilentSamples = maxSilenceDuration * sampleRate;
    
    inBuffer.reserve(maxChunkDuration * 1.25);
    transcriberCount = 0;
    clientTranscrPos = 0;
    acceptInput = false;
    isDone = false;
}

/*****************************************************************************
 Set silence parameters for the addSoundBufferSil(ofSoundBuffer&) method.

 @param thresh The minimum RMS a buffer must attain be considered not-silent
 @param duration The number of seconds of silence allowed before a chunk is
                 sent off to the Cloud Speech API.
 */
void ofxGoogleTranscriber::setSilenceThreshold(float thresh, float duration) {
    silenceThreshold = thresh;
    maxSilenceDuration = duration;
    maxSilentSamples = maxSilenceDuration * sampleRate;
}

/*****************************************************************************
 Signal beginning of sound input. Transcription will begin when input is provided
 via addSoundBuffer(ofSoundBuffer&) or addSpeechBuffer(ofSpeechBuffer&)

 @param lang Optional: language code, eg "da-DK".
 @param sampRate Optional: Defaults to 441000
 */
void ofxGoogleTranscriber::beginInput(string lang) {
    langCode = lang;
    acceptInput = true;
    isDone = false;
    
    transcripts.str("");
    
    clientTranscrPos = 0;
    silentSampleCount = 0;
    inBuffer.clear();
    ofLogNotice("ofxGoogleTranscriber") << "Ready for input";
}
void ofxGoogleTranscriber::beginInput() {
    beginInput(langCode);
}

/*****************************************************************************
 Enqueue soundbuffer to be transcribed. Performs simple silence detection to reduce
 splitting utterances across request boundaries.
 Need to call beginInput first for buffer to be processed.

 @param soundBuffer SoundBuffer to add, using sample rate and language from beginInput call.
 */
void ofxGoogleTranscriber::addSoundBufferSil(ofSoundBuffer& soundBuffer) {
    if (!acceptInput)
        return;

    float rms = 0;
    unsigned numSamples = soundBuffer.getNumFrames();
    
    for (int i = 0; i < numSamples; i++) {
        float sample = soundBuffer.getSample(i, 0);
        rms += sample * sample;
        inBuffer.push_back(getPCM(sample));
    }
    rms /= numSamples;
    rms = sqrt(rms);
    
    if (rms <= silenceThreshold)
        silentSampleCount += numSamples;
    
    if (inBuffer.size() >= maxChunkSamples or silentSampleCount >= maxSilentSamples)
        flushInBuffer();
}

/*****************************************************************************
 Enqueue soundbuffer to be transcribed.
 Need to call beginInput first for buffer to be processed.
 
 @param soundBuffer SoundBuffer to add, using sample rate and language from beginInput call.
 */
void ofxGoogleTranscriber::addSoundBuffer(ofSoundBuffer& soundBuffer) {
    if (!acceptInput)
        return;
    
    for (int i = 0; i < soundBuffer.getNumFrames(); i++)
        inBuffer.push_back(getPCM(soundBuffer.getSample(i, 0)));

    if (inBuffer.size() >= maxChunkSamples)
        flushInBuffer();
}


/*****************************************************************************
 Signal there is no more audio input to process and transcript should be finalized.
 */
void ofxGoogleTranscriber::endOfInput() {
    acceptInput = false;
    if (inBuffer.size())
        flushInBuffer();
    thread finishWaiter(&ofxGoogleTranscriber::waitForTranscribers, this);
    finishWaiter.detach();
}

/*****************************************************************************
 Get the "full" transcript. Note that the transcript is not guaranteed to 
 be complete until getIsDone() returns true.

 @return The "full" transcript.
 */
string ofxGoogleTranscriber::getTranscript() {
    lock_guard<mutex> guard(transcriptsMutex);
    return transcripts.str();
}

/*****************************************************************************
 Extract any transcripts that are new to the client, also indicates via return bool
 whether there is a new transcript since the client last checked.

 @param outString Pointer to the std::string where the new transcripts should be placed.
 @return True if there is a new transcript, false if not.
 */
bool ofxGoogleTranscriber::getNewTranscript(string* outString) {
    lock_guard<mutex> guard(transcriptsMutex);
    string transcript = transcripts.str();
    if (clientTranscrPos == transcript.length()) {
        *outString = "";
        return false;
    }
    *outString = transcript.substr(clientTranscrPos);
    clientTranscrPos = transcript.length();
    return true;
}

/*****************************************************************************
 Returns a new transcript if there is one, otherwise returns an empty string.
 */
string ofxGoogleTranscriber::getNewTranscript() {
    lock_guard<mutex> guard(transcriptsMutex);
    string transcript = transcripts.str();
    string retString = transcript.substr(clientTranscrPos);
    clientTranscrPos = transcript.length();
    return retString;
}


/*****************************************************************************
 If true, then the result of getTranscript() is final.
 */
bool ofxGoogleTranscriber::getIsDone(){
    return isDone;
}
