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
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <ctime>
#include <cstdlib>
#include <atomic>
#include <csignal>

const std::string PROJECT_ID = "babybeacon-2025";
const std::string API_KEY = "AIzaSyD_VEzYSS6iYYOMX2TeUkTJeWESkLjmWU0";
const std::string DEVICE_ID = "000000000";
const std::string BASE_URL = "https://firestore.googleapis.com/v1/projects/" + PROJECT_ID + "/databases/(default)/documents";

std::string CASCADE_PATH = "/home/raspberry/BabyEmotionRecognition/model/haarcascade_frontalface_alt2.xml";
std::string APP = "Baby Emotion Recognition Software";
std::string MODEL_PATH = "/home/raspberry/BabyEmotionRecognition/model/tensorflow_model.pb";  // Updated to match your model
using json = nlohmann::json;

std::atomic<bool> scanningActive(false);
std::atomic<bool> stopScanning(false);
int lastRideId = 0;
std::string scanningBaby = "";
std::string username = "";

std::string previous_sympton = "";
std::string previous_emotion = "";
int previous_acc = 0; 

bool flag = false;

std::string getCurrentTimestamp() {
    std::time_t now = std::time(nullptr);
    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return std::string(buf);
}

std::string getDeviceField(const std::string& field) {
    std::string url = BASE_URL + "/devices/" + DEVICE_ID + "?key=" + API_KEY;
    auto response = cpr::Get(cpr::Url{url});
    
    if (response.status_code == 200) {
        json jsonData = json::parse(response.text);
        
        if (jsonData.contains("fields") && jsonData["fields"].contains(field)) {
            if (jsonData["fields"][field].contains("stringValue")) {
                return jsonData["fields"][field]["stringValue"];
            } else if (jsonData["fields"][field].contains("integerValue")) {
                return std::to_string(jsonData["fields"][field]["integerValue"].get<int>());
            }
        }
    }
    std::cerr << "⚠️ Failed to fetch " << field << " | Status Code: " << response.status_code << " | Response: " << response.text << std::endl;
    return "";
}

std::string getUserField(const std::string& field) {
    if (username.empty()) {
        std::cerr << "⚠️ Cannot fetch " << field << ". Username not set.\n";
        return "";
    }

    std::string url = BASE_URL + "/users/" + username + "?key=" + API_KEY;
    auto response = cpr::Get(cpr::Url{url});

    if (response.status_code == 200) {
        json jsonData = json::parse(response.text);

        if (jsonData.contains("fields") && jsonData["fields"].contains(field)) {
            auto fieldValue = jsonData["fields"][field];

            if (fieldValue.contains("stringValue")) {
                return fieldValue["stringValue"].get<std::string>();  // Return as string
            } 
            if (fieldValue.contains("integerValue")) {
                return std::to_string(fieldValue["integerValue"].get<int>());  // Convert int to string
            }
        }
    }

    std::cerr << "⚠️ Failed to fetch " << field << " from users | Status Code: " 
              << response.status_code << " | Response: " << response.text << std::endl;
    return "";
}

int fetchLastRideId() {
    std::string url = BASE_URL + "/users/" + username + "?key=" + API_KEY;
    auto response = cpr::Get(cpr::Url{url});

    std::cout << "📡 Fetching last ride ID from: " << url << std::endl;
    std::cout << "🔍 Raw Firestore Response: " << response.text << std::endl;

    if (response.status_code == 200) {
        json userJson = json::parse(response.text);

        if (userJson.contains("fields") && userJson["fields"].contains("last_ride_Id")) {
            auto lastRideField = userJson["fields"]["last_ride_Id"];

            try {
                // Force Firestore to treat `last_ride_Id` as a string
                if (lastRideField.contains("stringValue")) {
                    int rideId = std::stoi(lastRideField["stringValue"].get<std::string>());
                    std::cout << "✅ last_ride_Id (retrieved as string, converted to int): " << rideId << std::endl;
                    return rideId;
                }
            } catch (const std::exception& e) {
                std::cerr << "❌ Error converting last_ride_Id to int: " << e.what() << std::endl;
            }
        }
    }

    std::cerr << "⚠️ Failed to fetch last ride ID! Defaulting to 0.\n";
    return 0;
}
void uploadScanToRide(int rideId, int scanCount) {
    // Firestore URL
    std::string url = BASE_URL + "/devices/" + DEVICE_ID + "/rides/ride_" + std::to_string(rideId) + "?key=" + API_KEY;

    // Fetch ride document
    auto response = cpr::Get(cpr::Url{url});
    if (response.status_code != 200) {
        std::cerr << "❌ Failed to fetch ride document! Response: " << response.text << std::endl;
        return;
    }

    json rideData = json::parse(response.text);
    json updatedRideData;
    if (rideData.contains("fields")) {
        updatedRideData["fields"] = rideData["fields"];
    }

    // Prepare scan structure
    std::string scanKey = "scan" + std::to_string(scanCount);

    updatedRideData["fields"][scanKey] = {
        {"mapValue", {
            {"fields", {
                {"accuracy", { {"integerValue", previous_acc} }},
                {"emotion", { {"stringValue", previous_emotion} }},
                {"emotion_sound", { {"stringValue", previous_sympton} }}
            }}
        }}
    };

    // Upload
    auto patchResponse = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{updatedRideData.dump()}
    );

    /*if (patchResponse.status_code == 200) {
        std::cout << "📡 Scan " << scanCount << " added: " << selectedEmotion << " + " << selectedSound << " Accuracy: " << accuracy << "%\n";
    } else {
        std::cerr << "❌ Failed to upload scan " << scanCount << "! Response: " << patchResponse.text << std::endl;
    }*/
}

void uploadScanToRide_Old(int rideId, int scanCount) {
    // Emotion-Sound Mapping (no optional/none sounds hard-coded)
    std::map<std::string, std::vector<std::string>> emotionSoundMap = {
        {"Angry", {"Bellypain", "Needs burping", "Discomfort", "Hungry", "Tired", "Cold/Hot", "Scared", "Lonely"}},
        {"Disgust", {"Bellypain", "Needs burping", "Discomfort", "Hungry", "Tired", "Cold/Hot", "Lonely"}},
        {"Fear", {"Scared", "Lonely", "Cold/Hot", "Discomfort", "Needs burping"}},
        {"Happy", {}},  // Will use neutral default
        {"Sad", {"Lonely", "Scared", "Bellypain", "Needs burping", "Discomfort"}},
        {"Surprise", {"Scared", "Bellypain", "Cold/Hot", "Discomfort", "Hungry"}},
        {"Neutral", {"Hungry", "Tired", "Needs burping"}}
    };

    // Weighted Randomness for Emotion Selection
    std::vector<std::pair<std::string, int>> emotionWeights = {
        {"Neutral", 50},
        {"Happy", 30},
        {"Sad", 10},
        {"Angry", 5},
        {"Fear", 3},
        {"Disgust", 1},
        {"Surprise", 1}
    };

    int randVal = rand() % 100;
    int cumulative = 0;
    std::string selectedEmotion = "Neutral";

    for (const auto& pair : emotionWeights) {
        cumulative += pair.second;
        if (randVal < cumulative) {
            selectedEmotion = pair.first;
            break;
        }
    }

    // Choose sound:
    std::string selectedSound;
    const auto& sounds = emotionSoundMap[selectedEmotion];

    int soundChance = rand() % 100;
    if (soundChance < 10) {
        // 10% chance → Optional sound
        selectedSound = "Optional";
    } else if (!sounds.empty()) {
        selectedSound = sounds[rand() % sounds.size()];
    } else {
        // No mapped sounds → Assign default neutral sound
        selectedSound = "Calm";
    }

    int accuracy = rand() % 21 + 80;  // 80 to 100%

    // Firestore URL
    std::string url = BASE_URL + "/devices/" + DEVICE_ID + "/rides/ride_" + std::to_string(rideId) + "?key=" + API_KEY;

    // Fetch ride document
    auto response = cpr::Get(cpr::Url{url});
    if (response.status_code != 200) {
        std::cerr << "❌ Failed to fetch ride document! Response: " << response.text << std::endl;
        return;
    }

    json rideData = json::parse(response.text);
    json updatedRideData;
    if (rideData.contains("fields")) {
        updatedRideData["fields"] = rideData["fields"];
    }

    // Prepare scan structure
    std::string scanKey = "scan" + std::to_string(scanCount);

    updatedRideData["fields"][scanKey] = {
        {"mapValue", {
            {"fields", {
                {"accuracy", { {"integerValue", accuracy} }},
                {"emotion", { {"stringValue", selectedEmotion} }},
                {"emotion_sound", { {"stringValue", selectedSound} }}
            }}
        }}
    };

    // Upload
    auto patchResponse = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{updatedRideData.dump()}
    );

    if (patchResponse.status_code == 200) {
        std::cout << "📡 Scan " << scanCount << " added: " 
                  << selectedEmotion << " + " << selectedSound 
                  << " Accuracy: " << accuracy << "%\n";
    } else {
        std::cerr << "❌ Failed to upload scan " << scanCount << "! Response: " << patchResponse.text << std::endl;
    }
}

void updateLastRideId(int rideId) {
    std::string url = BASE_URL + "/users/" + username + "?updateMask.fieldPaths=last_ride_Id&key=" + API_KEY;

    // Convert integer to string before uploading
    json payload = {
        {"fields", { 
            {"last_ride_Id", { {"stringValue", std::to_string(rideId)} }} 
        }}
    };

    auto response = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{payload.dump()}
    );

    if (response.status_code == 200) {
        std::cout << "✅ Successfully updated last_ride_Id to " << rideId << " (stored as string in Firestore).\n";
    } else {
        std::cerr << "❌ Failed to update last_ride_Id! Status Code: " << response.status_code 
                  << " | Response: " << response.text << "\n";
    }
}

void updateBabyRides(int rideId) {
    if (scanningBaby.empty()) return;

    std::string url = BASE_URL + "/users/" + username + "/baby/" + scanningBaby + "?key=" + API_KEY;

    // 🔍 Fetch the current baby document
    auto response = cpr::Get(cpr::Url{url});

    if (response.status_code != 200) {
        std::cerr << "❌ Failed to fetch baby document! Response: " << response.text << std::endl;
        return;
    }

    // 📜 Parse the existing baby data
    json babyData = json::parse(response.text);

    // ✅ Preserve all existing fields
    json updatedBabyData;
    if (babyData.contains("fields")) {
        updatedBabyData["fields"] = babyData["fields"]; // Copy all fields
    }

    // 🆕 Prepare new ride entry
    std::string rideString = "ride" + std::to_string(rideId);

    // 📝 Append the new ride to the rides array
    if (updatedBabyData["fields"].contains("rides") && updatedBabyData["fields"]["rides"].contains("arrayValue")) {
        updatedBabyData["fields"]["rides"]["arrayValue"]["values"].push_back({ {"stringValue", rideString} });
    } else {
        updatedBabyData["fields"]["rides"] = {
            {"arrayValue", { {"values", { { {"stringValue", rideString} } } } } }
        };
    }

    // 🚀 Re-upload the full modified baby document
    auto patchResponse = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{updatedBabyData.dump()}
    );

    if (patchResponse.status_code == 200) {
        std::cout << "👶 Ride " << rideString << " added to baby profile successfully.\n";
    } else {
        std::cerr << "❌ Failed to update baby rides! Response: " << patchResponse.text << std::endl;
    }
}

void setDeviceStatus(const std::string& status) {
    std::string url = BASE_URL + "/devices/" + DEVICE_ID + "?updateMask.fieldPaths=status&key=" + API_KEY;
    json payload = {
        {"fields", {
            {"status", { {"stringValue", status} }}
        }}
    };

    auto response = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{payload.dump()}
    );

    if (response.status_code == 200) {
        std::cout << "🔌 Device status updated to '" << status << "'\n";
    } else {
        std::cerr << "❌ Failed to update device status! " << response.text << std::endl;
    }
}

void createNewRide(int rideId) {
    std::string url = BASE_URL + "/devices/" + DEVICE_ID + "/rides/ride_" + std::to_string(rideId) + "?key=" + API_KEY;

    json payload = {
        {"fields", {
            {"start_time", { {"stringValue", getCurrentTimestamp()} }},
            {"end_time", { {"stringValue", ""} }},
            {"responses", {  // 🔥 Store responses as a map
                {"mapValue", {
                    {"fields", {  // Placeholder for future response entries
                        {"example_response", { {"stringValue", "neutral"} }}  // Example entry
                    }}
                }}
            }}
        }}
    };

    auto response = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{payload.dump()}
    );

    if (response.status_code == 200) {
        std::cout << "✅ Ride " << rideId << " created successfully with response tracking.\n";
    } else {
        std::cerr << "❌ Failed to create ride! Response: " << response.text << std::endl;
    }
}

void endRide(int rideId) {
    std::string url = BASE_URL + "/devices/" + DEVICE_ID + "/rides/ride_" + std::to_string(rideId) + "?key=" + API_KEY;
    
    // Fetch existing ride data
    auto response = cpr::Get(cpr::Url{url});
    if (response.status_code == 200) {
        json rideJson = json::parse(response.text);

        // Ensure "fields" exists
        if (!rideJson.contains("fields")) {
            rideJson["fields"] = json::object();
        }

        // Add or update the end_time field
        rideJson["fields"]["end_time"] = { {"stringValue", getCurrentTimestamp()} };

        // Send the updated data back to Firestore
        cpr::Patch(cpr::Url{url}, cpr::Header{{"Content-Type", "application/json"}}, cpr::Body{rideJson.dump()});

        // Update last ride ID
        updateLastRideId(rideId);
    } else {
        std::cerr << "❌ Failed to fetch ride data before ending ride! Status Code: " << response.status_code << std::endl;
    }
}

void scanningWorkflow() {
    //while (true) {
        if (scanningActive.load()) {
            std::cout << "🚀 Scanning started..." << std::endl;

            username = getDeviceField("username");
            std::cout << "👤 Username: " << username << std::endl;

            lastRideId = fetchLastRideId() + 1;
            std::cout << "🆕 New Ride ID: " << lastRideId << " (int)" << std::endl;

            scanningBaby = getUserField("scanning_baby");
            std::cout << "👶 Scanning Baby: " << scanningBaby << std::endl;

            // Create a new ride in the device's rides subcollection
            createNewRide(lastRideId);

            // Update the baby's rides array with the new ride
            updateBabyRides(lastRideId);

            std::cout << "🚀 Ride " << lastRideId << " started.\n";

            for (int scanCount = 1; !stopScanning.load(); ++scanCount) {
                std::cout << "📡 Uploading scan " << scanCount << "...\n";
                // Ensure scans are uploaded under the ride in the `rides` subcollection of the device
                if (flag == true){
                    uploadScanToRide(lastRideId, scanCount);
                }

                std::this_thread::sleep_for(std::chrono::seconds(5));
            }

            endRide(lastRideId);
            scanningActive.store(false);
            stopScanning.store(false);
            std::cout << "✅ Ride " << lastRideId << " ended.\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Status: " << scanningActive.load() << " *\n";
    //}
}

void signalHandler(int signum) {
    std::cout << "\n🔌 Caught signal (" << signum << "). Shutting down...\n";
    setDeviceStatus("off");
    exit(signum);
}

void monitorDeviceStatus() {
    //while (true) {
        std::string status = getDeviceField("status");
        std::cout << "🔍 Current Status: " << status << std::endl;
        
        if (status == "scanning" && !scanningActive.load()) {
            scanningActive.store(true);
            stopScanning.store(false);
            std::cout << "🚀 Status changed to SCANNING. Starting scan process..." << std::endl;
        } else if (status == "idle" && scanningActive.load()) {
            stopScanning.store(true);
            std::cout << "🛑 Status changed to IDLE. Stopping scan process..." << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
   // }
}

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
    srand(static_cast<unsigned>(time(nullptr)));
    while(true){
        srand(static_cast<unsigned>(time(nullptr)));

        std::promise<std::string> symptom;
        std::future<std::string> fut_sym = symptom.get_future();

        std::promise<std::string> emotion;
        std::future<std::string> fut_emo = emotion.get_future();

        std::thread symptomDetect(SymptomDetection, std::move(symptom));
        std::thread emotionDetect(EmotionDetection, std::move(emotion));

        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);
        setDeviceStatus("idle");
        std::thread monitorThread(monitorDeviceStatus);
        std::thread scanThread(scanningWorkflow);


        std::cout << "Monitor Thread Start ....\n";
        monitorThread.join();
        std::cout << "Scan Thread Start ....\n";
        scanThread.join();
        std::cout << "Symptom Detect Thread Start ....\n";
        symptomDetect.join();
        std::cout << "Emotion Detect Thread Start ....\n";
        emotionDetect.join();

        std::cout << "------------------------------------------------------------\n";
        std::string predicted_symptom; 
        try{
            predicted_symptom = fut_sym.get();
        }catch (const std::exception& e){
            std::cout << "Symptom Empty\n";
        }
        try{
            std::string detected_emotion = fut_emo.get();
            std::vector<std::string> emotion_parsed = split_string(detected_emotion);
  
            if ( (emotion_parsed[0] != previous_emotion) || (predicted_symptom != previous_sympton)){
                flag = __cpp_lib_allocator_traits_is_always_equal; 
                previous_emotion = emotion_parsed[0];
                previous_sympton = predicted_symptom;
                previous_acc = std::stoi(emotion_parsed[1]);
                std::cout << "Emotion = " << previous_emotion<< std::endl;
                std::cout << "Symptom = " << previous_sympton << std::endl; 
                std::cout << "Emotion Accuracy = " << previous_acc << std::endl; 
                flag = true;  
            } else{
                flag = false;
            }
        }catch (const std::exception& e){
            std::cout << "Emotion Empty Frame\n" ;
            flag = false;
        }

        std::cout << "------------------------------------------------------------\n";

    }
    
    return 0; 
}
