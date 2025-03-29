#include <iostream>
#include <vector>
#include <aubio/aubio.h>
#include <sndfile.h>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <limits>
#include <string> 

#define SAMPLE_RATE 22050  // Match Librosa
#define FRAME_SIZE 2048    // Match n_fft in Librosa
#define HOP_SIZE 512       // Match hop_length in Librosa
#define N_FILTERS 42       // Match Librosa Mel filters (N_MFCC + 2)
#define N_MFCC 40          // Number of MFCC coefficients

//namespace fs = std::filesystem;

#define CWD "/home/raspberry/BabyEmotionRecognition/"

#define ADP_FILENAME "/home/raspberry/BabyEmotionRecognition/recording.wav"
#define MFCC_FILE "/home/raspberry/BabyEmotionRecognition/output/mfcc_features.txt"
#define INPUT_FILE "/home/raspberry/BabyEmotionRecognition/output/predicted_symptom.txt"

int export_mfccFile(std::vector<float> mfcc_features, const char* txtFile);
std::vector<float> extract_mfcc(const char* filename);
std::string readPredication(const char* filename);
