
#Real-Time Facial Emotion Recognition for Baby Beacon

## Introduction
The application will capture video from the laptop camera and recognise the facial expression as one of 7 emotions (eg. happy, sad, angry, disgust, surprise, fear, neutral) displayed by the person's face. The user will be able to see the facial emotion prediction displayed on their screen in real-time.


## Dependencies for Running Locally (this project was run and tested on Mac)
* cmake >= 3.7
* make >= 4.1 (Linux, Mac), 3.81 (Windows)
* gcc/g++ >= 5.4
* OpenCV == 4.3.0 (other versions may work but they are untested)
* tensorflow <= 1.15 (for python notebooks only)

## Basic Build Instructions
1. Clone this repo.
2. Make a build directory in the top level directory: `mkdir build && cd build`
3. Compile: `cmake .. && make`
4. Run it: `./emotion_detector`.