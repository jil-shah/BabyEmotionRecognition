#include "../include/FaceDetection.h"

FaceDetection:: FaceDetection(const std::string& cascadePath) {

    //load the cascade file for the pre-trained machine learning models for face detection
    if(!faceCascade.load(cascadePath)){
        throw std::runtime_error("Error loading haar cascade file");
    }
}

std::vector<cv::Rect> FaceDetection::detectFaces(const cv::Mat& frame){
    std::vector<cv::Rect> Faces;
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    faceCascade.detectMultiScale(gray, Faces, 1.1, 5, cv::CASCADE_SCALE_IMAGE, cv::Size(30,30));
    return Faces;
}

void FaceDetection::drawFace(cv::Mat& frame, const cv::Rect& face){
    cv::rectangle(frame, face, cv::Scalar(0,255,0), 2);
}

void FaceDetection::drawFace(cv::Mat& frame, const cv::Rect& face, const std::string& label) {
    cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, label, cv::Point(face.x, face.y - 10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
}