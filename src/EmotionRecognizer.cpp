#include "../include/EmotionRecognizer.h"


EmotionRecognizer::EmotionRecognizer(const std::string& modelPath)
    : network(cv::dnn::readNet(modelPath)), // Load the TensorFlow model
      classid_to_string({{0, "Angry"}, 
                         {1, "Disgust"}, 
                         {2, "Fear"}, 
                         {3, "Happy"}, 
                         {4, "Sad"}, 
                         {5, "Surprise"}, 
                         {6, "Neutral"}}) // Create a map from class id to the class labels
{}

std::vector<std::string> EmotionRecognizer::predict(cv::Mat& faceROI) {
    std::vector<std::string> emotion_prediction;

    // Convert to grayscale (needed for the model)
    cv::Mat gray_image;
    cv::cvtColor(faceROI, gray_image, cv::COLOR_BGR2GRAY);

    // Resize the ROI to match the model input size (48x48)
    cv::Mat resized_image;
    cv::resize(gray_image, resized_image, cv::Size(48, 48));

    // Normalize pixel values to [0,1]
    resized_image.convertTo(resized_image, CV_32FC3, 1.f / 255);

    // Convert to blob format for DNN input
    cv::Mat blob = cv::dnn::blobFromImage(resized_image);

    // Set model input
    this->network.setInput(blob);

    // Forward pass
    cv::Mat prob = this->network.forward();

    // Sort probabilities
    cv::Mat sorted_probabilities;
    cv::Mat sorted_ids;
    cv::sort(prob.reshape(1, 1), sorted_probabilities, cv::SORT_DESCENDING);
    cv::sortIdx(prob.reshape(1, 1), sorted_ids, cv::SORT_DESCENDING);

    // Get top probability and class ID
    float top_probability = sorted_probabilities.at<float>(0);
    int top_class_id = sorted_ids.at<int>(0);

    // Map classId to class label
    std::string class_name = this->classid_to_string.at(top_class_id);
    std::string result_string = class_name + ": " + std::to_string(top_probability * 100) + "%";

    // Append result
    emotion_prediction.push_back(result_string);

    return emotion_prediction;
}
