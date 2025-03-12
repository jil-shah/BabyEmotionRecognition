#include <iostream>
#include <vector>
#include <aubio/aubio.h>
#include <sndfile.h>
#include <fstream>
//#include <python3.12/Python.h>
#include <filesystem>
//#include "../librosa/librosa.h"
#include <cmath>
#include <limits>

#define SAMPLE_RATE 22050  // Match Librosa
#define FRAME_SIZE 2048    // Match n_fft in Librosa
#define HOP_SIZE 512       // Match hop_length in Librosa
#define N_FILTERS 42       // Match Librosa Mel filters (N_MFCC + 2)
#define N_MFCC 40          // Number of MFCC coefficients

#define ADP_FILENAME "/home/ghosttt/Baby-Beacon-Sound-Emotion/data/testing_data/belly_pain.wav"
#define MFCC_FILE "/home/ghosttt/Baby-Beacon-Sound-Emotion/output/mfcc_features.txt"
#define SITE_PACKAGES "sys.path.append('/home/ghosttt/venv/lib/python3.12/site-packages')"
#define PYTHON_SCRIPT "sys.path.append('/home/ghosttt/Baby-Beacon-Sound-Emotion/python')"
#define PY_FILE "testingModel"

std::vector<float> extract_mfcc2(const char* filename);
int export_mfccFile(std::vector<float> mfcc_features, const char* txtFile);
//int pyPredict_Emotions();

std::vector<float> extract_mfcc(const char* filename);