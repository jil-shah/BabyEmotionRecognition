#include <iostream>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <chrono>
#include <atomic>

using json = nlohmann::json;

const std::string PROJECT_ID = "babybeacon-2025";
const std::string API_KEY    = "AIzaSyD_VEzYSS6iYYOMX2TeUkTJeWESkLjmWU0";

const std::string DEVICE_ID  = "222222222";

const std::string BASE_URL   = "https://firestore.googleapis.com/v1/projects/" 
                               + PROJECT_ID + "/databases/(default)/documents";

// The user & baby we want to reference:
std::string username     = "bob";
std::string scanningBaby = "Tubby";
int lastRideId = 0; 

std::string toTimestamp(time_t t)
{
    // Convert to UTC
    std::tm* gmt = std::gmtime(&t);
    char buf[30];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", gmt);
    return std::string(buf);
}

// Get user’s last_ride_Id (stored as stringValue) from Firestore
int fetchLastRideId()
{
    std::string url = BASE_URL + "/users/" + username + "?key=" + API_KEY;
    auto response   = cpr::Get(cpr::Url{url});

    if (response.status_code == 200) {
        json userJson = json::parse(response.text);
        if (userJson.contains("fields") && userJson["fields"].contains("last_ride_Id"))
        {
            auto &val = userJson["fields"]["last_ride_Id"];
            if (val.contains("stringValue"))
            {
                try {
                    return std::stoi(val["stringValue"].get<std::string>());
                }
                catch (...) {
                    std::cerr << "❌ Error parsing last_ride_Id.\n";
                }
            }
        }
    }
    std::cerr << "⚠️ Failed to fetch last ride ID, defaulting to 0.\n";
    return 0;
}

// Update user’s last_ride_Id in Firestore
void updateLastRideId(int rideId)
{
    std::string url = BASE_URL 
        + "/users/" + username 
        + "?updateMask.fieldPaths=last_ride_Id&key=" + API_KEY;

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
        std::cout << "✅ Updated last_ride_Id to " << rideId << "\n";
    } else {
        std::cerr << "❌ Failed to update last_ride_Id: " << response.text << "\n";
    }
}

// Add the new ride to the baby’s array of rides
void updateBabyRides(int rideId)
{
    // If for some reason scanningBaby is empty, skip
    if (scanningBaby.empty()) return;

    // Fetch baby doc
    std::string url = BASE_URL 
        + "/users/" + username 
        + "/baby/" + scanningBaby 
        + "?key=" + API_KEY;

    auto response = cpr::Get(cpr::Url{url});
    if (response.status_code != 200) {
        std::cerr << "❌ Failed to fetch baby doc: " << response.text << "\n";
        return;
    }

    json babyData = json::parse(response.text);
    json updatedBabyData;
    if (babyData.contains("fields")) {
        updatedBabyData["fields"] = babyData["fields"];
    }

    // Add ride string to "rides" array
    std::string rideString = "ride" + std::to_string(rideId);
    if (updatedBabyData["fields"].contains("rides") &&
        updatedBabyData["fields"]["rides"].contains("arrayValue"))
    {
        updatedBabyData["fields"]["rides"]["arrayValue"]["values"].push_back(
            { {"stringValue", rideString} }
        );
    } 
    else {
        updatedBabyData["fields"]["rides"] = {
            {"arrayValue", {
                {"values", { {{"stringValue", rideString}} }}
            }}
        };
    }

    // Patch baby doc
    auto patchResponse = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{updatedBabyData.dump()}
    );

    if (patchResponse.status_code == 200) {
        std::cout << "👶 Added ride to baby doc: " << rideString << "\n";
    } else {
        std::cerr << "❌ Failed to patch baby doc: " << patchResponse.text << "\n";
    }
}

// Create a new ride document in the device’s rides/ subcollection
// Here we allow passing the desired startTime explicitly
void createNewRide(int rideId, const std::string &startTime)
{
    std::string url = BASE_URL 
        + "/devices/" + DEVICE_ID 
        + "/rides/ride_" + std::to_string(rideId) 
        + "?key=" + API_KEY;

    json payload = {
        {"fields", {
            {"start_time", { {"stringValue", startTime} }},
            {"end_time",   { {"stringValue", ""} }},
            // example "responses" map field
            {"responses", { 
                {"mapValue", {
                    {"fields", {
                        {"example_response", { {"stringValue", "neutral"} }}
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
        std::cout << "✅ Created ride_" << rideId << " (start_time=" << startTime << ")\n";
    } else {
        std::cerr << "❌ Failed to create ride_" << rideId 
                  << ": " << response.text << "\n";
    }
}

// End ride by setting end_time
void endRide(int rideId, const std::string &endTime)
{
    // fetch existing
    std::string url = BASE_URL 
        + "/devices/" + DEVICE_ID 
        + "/rides/ride_" + std::to_string(rideId) 
        + "?key=" + API_KEY;

    auto response = cpr::Get(cpr::Url{url});
    if (response.status_code != 200) {
        std::cerr << "❌ Could not fetch ride for endRide: " << response.text << "\n";
        return;
    }

    json rideJson = json::parse(response.text);
    if (!rideJson.contains("fields")) {
        rideJson["fields"] = json::object();
    }
    rideJson["fields"]["end_time"] = { {"stringValue", endTime} };

    auto patchResponse = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{rideJson.dump()}
    );

    if (patchResponse.status_code == 200) {
        std::cout << "✅ ride_" << rideId << " ended (end_time=" << endTime << ")\n";
    } else {
        std::cerr << "❌ Failed to end ride_" << rideId 
                  << ": " << patchResponse.text << "\n";
    }

    // Also update last_ride_Id in user doc
    updateLastRideId(rideId);
}

// Upload a single scan (reuses your logic).
// Weighted random emotion → optional or mapped sound → accuracy
void uploadScanToRide(int rideId, int scanCount)
{
    // Emotion-Sound Mapping
    static std::map<std::string, std::vector<std::string>> emotionSoundMap = {
        {"Angry",    {"Bellypain", "Needs burping", "Discomfort", "Hungry", 
                      "Tired", "Cold/Hot", "Scared", "Lonely"}},
        {"Disgust",  {"Bellypain", "Needs burping", "Discomfort", "Hungry", 
                      "Tired", "Cold/Hot", "Lonely"}},
        {"Fear",     {"Scared", "Lonely", "Cold/Hot", "Discomfort", "Needs burping"}},
        {"Happy",    {}},  // Will fall back to neutral
        {"Sad",      {"Lonely", "Scared", "Bellypain", "Needs burping", "Discomfort"}},
        {"Surprise", {"Scared", "Bellypain", "Cold/Hot", "Discomfort", "Hungry"}},
        {"Neutral",  {"Hungry", "Tired", "Needs burping"}}
    };

    // Weighted Randomness for Emotion
    static std::vector<std::pair<std::string,int>> emotionWeights = {
        {"Neutral",  50},
        {"Happy",    30},
        {"Sad",      10},
        {"Angry",    5 },
        {"Fear",     3 },
        {"Disgust",  1 },
        {"Surprise", 1 }
    };

    // 1) Pick random emotion
    int randVal = rand() % 100;
    int cumulative = 0;
    std::string selectedEmotion = "Neutral";
    for (auto &pair : emotionWeights) {
        cumulative += pair.second;
        if (randVal < cumulative) {
            selectedEmotion = pair.first;
            break;
        }
    }

    // 2) Choose sound from the mapping
    std::string selectedSound;
    auto &possibleSounds = emotionSoundMap[selectedEmotion];
    int soundChance = rand() % 100;
    if (soundChance < 10) {
        // 10% → Optional
        selectedSound = "Optional";
    } else if (!possibleSounds.empty()) {
        selectedSound = possibleSounds[rand() % possibleSounds.size()];
    } else {
        // No mapped sounds → neutral default
        selectedSound = "Calm";
    }

    // 3) Accuracy (80–100)
    int accuracy = 80 + (rand() % 21);

    // 4) Fetch existing ride doc
    std::string url = BASE_URL 
        + "/devices/" + DEVICE_ID 
        + "/rides/ride_" + std::to_string(rideId) 
        + "?key=" + API_KEY;

    auto response = cpr::Get(cpr::Url{url});
    if (response.status_code != 200) {
        std::cerr << "❌ Could not fetch ride_" << rideId 
                  << ": " << response.text << "\n";
        return;
    }

    json rideData = json::parse(response.text);
    json updatedRideData;
    if (rideData.contains("fields")) {
        updatedRideData["fields"] = rideData["fields"];
    }

    // 5) Prepare scan key
    std::string scanKey = "scan" + std::to_string(scanCount);
    updatedRideData["fields"][scanKey] = {
        {"mapValue", {
            {"fields", {
                {"accuracy",      { {"integerValue", accuracy} }},
                {"emotion",       { {"stringValue", selectedEmotion} }},
                {"emotion_sound", { {"stringValue", selectedSound} }}
            }}
        }}
    };

    // 6) Patch ride doc
    auto patchResponse = cpr::Patch(
        cpr::Url{url},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Body{updatedRideData.dump()}
    );

    if (patchResponse.status_code == 200) {
        std::cout << "📡 Scan " << scanCount << " → " 
                  << selectedEmotion << " + " << selectedSound 
                  << " (Accuracy=" << accuracy << "%)\n";
    } else {
        std::cerr << "❌ Failed to upload scan" << scanCount 
                  << ": " << patchResponse.text << "\n";
    }
}

// Returns a random ride length in minutes, following the distribution:
// 0–5 (10%), 5–10 (20%), 10–30 (40%), 30–60 (20%), 60–120 (10%)
int getRandomRideLengthMinutes()
{
    int r = rand() % 100; // 0..99
    if (r < 10) {
        // 0–5
        return (rand() % 6); 
    } else if (r < 30) {
        // 5–10
        return 5 + (rand() % 6);
    } else if (r < 70) {
        // 10–30
        return 10 + (rand() % 21);
    } else if (r < 90) {
        // 30–60
        return 30 + (rand() % 31);
    } else {
        // 60–120
        return 60 + (rand() % 61);
    }
}

// Choose a random time of day for the ride’s start
//   morning   6–11   (30%)
//   afternoon 12–17  (30%)
//   evening   18–21  (30%)
//   night     22–23  (10%)
// We just pick hour in that range + a random minute
void pickRandomDaypart(int &outHour, int &outMin)
{
    int r = rand() % 100; // 0..99
    outMin = rand() % 60;
    if (r < 30) {
        // morning
        outHour = 6 + (rand() % 6);  // 6..11
    } else if (r < 60) {
        // afternoon
        outHour = 12 + (rand() % 6); // 12..17
    } else if (r < 90) {
        // evening
        outHour = 18 + (rand() % 4); // 18..21
    } else {
        // night
        outHour = 22 + (rand() % 2); // 22..23
    }
}

// ---------------------------------------------------
// Main: Generate rides from 2025-01-01 to "today" 
// ---------------------------------------------------
int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    // 1) Find our last known rideID so we can keep incrementing
    lastRideId = fetchLastRideId();

    // 2) Construct start/end dates
    std::tm tmStart = {};
    tmStart.tm_year = 2025 - 1900; // 2025
    tmStart.tm_mon  = 1;           // February = 1
    tmStart.tm_mday = 14;          // 14th
    tmStart.tm_hour = 0;
    tmStart.tm_min  = 0;
    tmStart.tm_sec  = 0;
    time_t startDate = std::mktime(&tmStart);

    // “today” at midnight
    time_t now      = std::time(nullptr);
    std::tm* tmNow  = std::localtime(&now);
    tmNow->tm_hour  = 0;
    tmNow->tm_min   = 0;
    tmNow->tm_sec   = 0;
    time_t endDate  = std::mktime(tmNow);

    // 3) Loop day by day
    for (time_t dayT = startDate; dayT <= endDate; dayT += 24*3600)
    {
        // 30% skip day, 20% = 2 rides, 50% = 1 ride
        int skipRnd = rand() % 100; 
        if (skipRnd < 30) {
            continue; // skip day
        }

        int ridesToday = (skipRnd < 50) ? 1 : 2; 
        // Explanation: skipRnd<30 => skip; 30..49 => 1 ride (20%), 50..99 => 2 rides (50% leftover? Actually it's 50..99 => 2 rides => 50%).
        // If you want exactly 20% for 2 rides vs 50% for 1 ride, adjust logic as needed.

        // For each ride
        for (int i = 0; i < ridesToday; i++)
        {
            // 4) Choose random start hour/min
            int hour, minute;
            pickRandomDaypart(hour, minute);

            // Build a tm from dayT’s year/month/day, then set hour/min
            std::tm rideStartLocal = *std::localtime(&dayT);
            rideStartLocal.tm_hour = hour;
            rideStartLocal.tm_min  = minute;
            rideStartLocal.tm_sec  = 0;
            time_t rideStartTimeT  = std::mktime(&rideStartLocal);

            // 5) Choose random ride length
            int lengthMin = getRandomRideLengthMinutes();
            time_t rideEndTimeT = rideStartTimeT + (lengthMin * 60);

            // 6) We'll create a new ride
            ++lastRideId; 
            std::string startString = toTimestamp(rideStartTimeT);
            std::string endString   = toTimestamp(rideEndTimeT);

            createNewRide(lastRideId, startString);
            updateBabyRides(lastRideId);

            // 7) Number of scans = rideLength / 0.5 min (30s)
            int totalScans = (lengthMin * 60) / 30;  // integer
            for (int s = 1; s <= totalScans; s++) {
                uploadScanToRide(lastRideId, s);
            }

            // 8) End the ride & update user’s lastRideId
            endRide(lastRideId, endString);
        }
    }

    std::cout << "\n✅ Done populating rides.\n";
    return 0;
}