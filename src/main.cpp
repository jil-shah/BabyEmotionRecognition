#include <opencv2/opencv.hpp>
#include <iostream>

const std::string APP = "Baby Emotion Recognition Software";
int main(){
    std::cerr << "Baby Emotion Recognition Software Enabled" << std::endl;
    
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
    cap.set(cv::CAP_PROP_FPS, 30);  // Set FPS to 30 for smoother video
    // Use MJPEG codec for a better opencv backend
    cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G')); 

    while(true){
        //read a new frame
        cap >> frame;
        if(frame.empty()){
            std::cerr << "Error: Empty frame received" << std::endl;
            continue;
        }

        //display live video without processing
        cv::imshow(APP, frame);

        if(cv::waitKey(1) == 'q') break;

    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}