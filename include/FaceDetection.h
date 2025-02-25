#ifndef FACEDETECTION_H
#define FACEDETECTION_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
class FaceDetection {
public:
    FaceDetection(const std::string& cascadePath);
    std::vector<cv::Rect> detectFaces (const cv::Mat& frame);
    void drawFace(cv::Mat& frame, const cv::Rect& face);
    void drawFace(cv::Mat& frame, const cv::Rect& face, const std::string& label);

private:
    cv::CascadeClassifier faceCascade;
};

#endif