#include "../include/FaceDetection.h"
#include "../include/EmotionRecognizer.h"
#include "../include/Record_BabySounds.h"
#include "../include/ADProcess_BabySounds.h"
#include <opencv2/opencv.hpp>
#include <chrono>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include <string>
#include <thread>
#include <future>

std::string CASCADE_PATH = "/home/raspberry/BabyEmotionRecognition/model/haarcascade_frontalface_alt2.xml";
std::string APP = "Baby Emotion Recognition Software";
std::string MODEL_PATH = "/home/raspberry/BabyEmotionRecognition/model/tensorflow_model.pb";  // Updated to match your model

int SymptomDetection(std::promise<std::string> prom){

        
    record(); 
    std::vector<float> mfcc = extract_mfcc(ADP_FILENAME);
    std::cout << "MFCC Features extracted\n";
    export_mfccFile(mfcc, MFCC_FILE);
    
    std::string sound_symptom = readPredication(INPUT_FILE); // get the emotion from the text file created by Python Component
    
    prom.set_value(sound_symptom);
    return 0; 
}

int EmotionDetection(std::promise<std::string> prom){
    
    std::cerr << "Baby Emotion Recognition Software Enabled" << std::endl;
    FaceDetection FaceDetection(CASCADE_PATH);
    EmotionRecognizer EmotionRecognizer(MODEL_PATH);
    
    // initialize the video frame
    cv::Mat frame;

    // initialize the video capture object
    cv::VideoCapture cap(0, cv::CAP_V4L2);
    if(!cap.isOpened()) {
        std::cerr << "Error: Unable to open web camera" << std::endl;
        return -1;
    }

    // create the window
    cv::namedWindow(APP);

    //adjust frame due to slow VM processing time
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);  // Try 1280x720
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_FPS, 5);  // Set FPS to 30 for smoother video
    // Use MJPEG codec for a better opencv backend
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); 
        
    //read a new frame
    cap >> frame;
    if(frame.empty()){
        std::cerr << "Error: Empty frame received" << std::endl;
        exit(1);
    }
    
    //detect faces
    auto faces = FaceDetection.detectFaces(frame);
    for(const auto& face : faces) {
        cv::Mat faceROI = frame(face);
        std::vector<std::string> emotions = EmotionRecognizer.predict(faceROI);
        FaceDetection.drawFace(frame, face, emotions[0]);
        prom.set_value(emotions[0]);
        break;
    }
    cap.release();
    
    return 0;
}

std::vector<std::string> split_string(std::string input){
    
    size_t pos = input.find(":");
    
    if(pos != std::string::npos){
    
        std::string emotion = input.substr(0,pos);
        std::string accuracy = input.substr(pos+2);

        return {emotion, accuracy}; 

        
    }else{
        std::cout << "Cannot Parse No Colon Found\n";
        return {input};
    }
    
    
    
}

int main(){
    
    while(true){
        std::promise<std::string> symptom;
        std::future<std::string> fut_sym = symptom.get_future();

        std::promise<std::string> emotion;
        std::future<std::string> fut_emo = emotion.get_future();

        std::thread symptomDetect(SymptomDetection, std::move(symptom));
        std::thread emotionDetect(EmotionDetection, std::move(emotion));
    
        symptomDetect.join();
        emotionDetect.join();
    
        
        
        
        std::cout << "------------------------------------------------------------\n";
        try{
            std::string predicted_symptom = fut_sym.get();
            std::cout << "Symptom = " << predicted_symptom << std::endl; 
        }catch (const std::exception& e){
            std::cout << "Symptom Empty\n" ;

        }
        try{
            std::string detected_emotion = fut_emo.get();
            std::vector<std::string> emotion_parsed = split_string(detected_emotion);
            std::cout << "Emotion = " << emotion_parsed[0] << std::endl;
            std::cout << "Emotion Accuracy = " << emotion_parsed[1] << std::endl;     

        }catch (const std::exception& e){
            std::cout << "Emotion Empty Frame\n" ;
        }

        std::cout << "------------------------------------------------------------\n";

    }
    
    return 0; 
}
