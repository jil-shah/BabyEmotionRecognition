#ifndef EMOTIONRECOGNIZER_H
#define EMOTIONRECOGNIZER_H

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <vector>
#include <string>
#include <unordered_map>

class EmotionRecognizer {
public:
    explicit EmotionRecognizer(const std::string& modelPath);
    std::vector<std::string> predict(cv::Mat& faceROI); // Updated to match Model.cpp

private:
    cv::dnn::Net network;
    std::unordered_map<int, std::string> classid_to_string;
};

#endif
