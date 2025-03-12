import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
import librosa
import numpy as np
import tensorflow as tf
import keras
import os
# CONSTANTS ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
"""
DIR = "C:\\Users\\shivt\\Documents\\GitHub\\Baby-Beacon-Sound-Emotion\\"
DIR = "C:\\Users\\shivt\\Code\\BabyBeacon\\Baby-Beacon-Sound-Emotion\\"
AUDIO_PATH = DIR+"data\\testing_data"
MODEL_PATH = DIR + "model\\"
"""
""" DIR = "/home/ghosttt/BabyEmotionRecognition/"
AUDIO_PATH = DIR+"/data/testing_data"
MODEL_PATH = DIR + "/model/" """

DIR = os.getcwd()
AUDIO_PATH = DIR+"/data/testing_data"
MODEL_PATH = DIR + "/model/"


SAMPLE_RATE = 22050  # Sample rate for librosa
DURATION = 6       # Duration of the audio file (seconds)
N_MFCC = 40          # Number of MFCCs to extract
EMOTIONS = ["belly_pain", "burping", "discomfort", "hungry", "tired"]

# LOAD KERAS MODEL --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
model = keras.models.load_model(MODEL_PATH+"Symptom_Detection_Model.keras") # ADD "augmented_" or "expanded_" to test different versions of the model

# PREDICT FROM TXT FILE
def predictFromFile(file_path):
    features = np.loadtxt(file_path, dtype=np.float64)
    #print(features) 

    # Reshape the features to match the input shape expected by the model
    features = np.expand_dims(features, axis=0)  # Add batch dimension
    
    # Make the prediction
    prediction = model.predict(features)
    
    # Get the predicted class
    predicted_class = np.argmax(prediction, axis=1)
    
    # Return the predicted emotion label
    return EMOTIONS[predicted_class[0]]
    


