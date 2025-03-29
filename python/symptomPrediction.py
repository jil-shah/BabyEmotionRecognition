import os
import numpy as np
from tensorflow import keras
import os
# CONSTANTS ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
DIR = "/home/raspberry/BabyEmotionRecognition"
AUDIO_PATH = DIR+"/data/testing_data"
MODEL_PATH = DIR + "/model/"

EMOTIONS = ["belly_pain", "burping", "discomfort", "hungry", "tired"]

# LOAD KERAS MODEL --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
model = keras.models.load_model(MODEL_PATH+"baby_emotions_model.h5") # ADD "augmented_" or "expanded_" to test different versions of the model

# PREDICT FROM TXT FILE
def predictFromFile(file_path):
    features = np.loadtxt(file_path, dtype=np.float64)
    
    print(features) 

    # Reshape the features to match the input shape expected by the model
    features = np.expand_dims(features, axis=0)  # Add batch dimension
    
    # Make the prediction
    prediction = model.predict(features)
    
    # Get the predicted class
    predicted_class = np.argmax(prediction, axis=1)
    
    # Return the predicted emotion label
    return EMOTIONS[predicted_class[0]]
    
