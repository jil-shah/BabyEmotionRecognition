#include "../include/Record_BabySounds.h"

int recordCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData){

    AudioData *data = (AudioData *)userData; 
    const short *input = (const short *)inputBuffer; 

    if(inputBuffer != nullptr){
        data->samples.insert(data->samples.end(),input, input+framesPerBuffer);
    }

    return paContinue;

}

int record(){

    Pa_Initialize(); // initalize portaudio 

    AudioData data;
    PaStream *stream; 

    Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, paInt16, REC_SAMPLE_RATE, FRAMES_PER_BUFFER, recordCallback, &data);

    std::cout << "Start Recording....\n"; 
    Pa_StartStream(stream);             // start recording 
    Pa_Sleep(RECORD_SECONDS * 1000);    // 10000 ms delay
    Pa_StopStream(stream);              // stop recording
    Pa_CloseStream(stream);
    Pa_Terminate();
    std::cout << "Finished Recording....\n";

    // Save audio as a .wav file using libsndfile library 
    SF_INFO sfinfo; 
    sfinfo.samplerate = REC_SAMPLE_RATE; 
    sfinfo.channels = NUM_CHANNELS;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16; 

    SNDFILE *FILE = sf_open(FILENAME, SFM_WRITE, &sfinfo);
    if(!FILE){
        std::cerr << "Error Opening Output File! \n";
        exit(1);
    }

    sf_write_short(FILE, data.samples.data(), data.samples.size());
    sf_close(FILE);

    std::cout << "Saved as " << FILENAME << "\n"; 

    return 0;
}


