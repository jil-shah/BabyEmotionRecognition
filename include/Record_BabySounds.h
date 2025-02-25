#include <opencv4/opencv2/opencv.hpp> // import opencv files 
#include <stdlib.h>
#include <iostream>
#include <portaudio.h>
#include <sndfile.h>
#include <vector>

#define REC_SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 1024
#define NUM_CHANNELS 1
#define RECORD_SECONDS 12
#define FILENAME "recording.wav"

/*
 * To compile and run: 
 *      > make
 *      > ./BabySounds
 */ 

// Create a struct for AudioData samples
struct AudioData{

    std::vector<short> samples; 

};

// Record Call Back Function Prototype
int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

// Record Audio Function Prototype
int record();


