# BabyBeacon Emotion Recognition
## BabyBeacon Overview
In today’s fast-paced society, parents often balance numerous responsibilities including work, errands and childcare, leading to increased stress and divided attention, especially during travel. The presence of a baby in the backseat presents an added level of distraction as parents feel compelled to monitor their child's behaviour, increasing the risk of an accident. Existing baby monitoring devices primarily focus on home environments and are not catered to the variability of a car's environment. Our project called ‘BabyBeacon’ aims to address this issue by implementing an in-car system that continuously monitors the baby’s behaviour and provides real-time alerts to the driver, allowing them to focus on the road. The potential end users of our system are individuals who frequently travel with infants or toddlers. This includes: parents, guardians, caregivers and child safety workers. The system will incorporate a wide range of technical concepts, including facial/movement recognition, sound detection, and a user interface, along with an API to facilitate data storage and machine learning integration.

Additionally, testing will be conducted under various driving conditions to validate both the performance and response time of the system. The funding for the project will be provided by the McMaster ECE Capstone budget. The primary risk is the system’s ability to accurately interpret the baby’s sounds and motion in the vehicle, as incorrect suggestions may further distract the driver. In conclusion, the project aims to reduce driver distraction and place their mind at ease by ensuring the needs and safety of the baby are monitored. 

## Repository Overview
This repository is a component of the Baby Beacon Project. It deals with recognizing different sounds a baby could make during a car ride. Using Tensorflow and Keras in Python, we have developed a model that would be able to recognize the following emotions based on different cries. 

- Belly Pain
- Needs Burping
- Discomfort
- Hungry
- Tired

## Training the AI Model
During the research phase, we found a few pre-trained models that work for Emotion Recognition in adults, but not many for infants. 
For Adult Emotion Recognition these models and datasets could be useful:
- [Github.com Voice Emotion Detector](https://github.com/crhung/Voice-Emotion-Detector)

### Dataset
After further research, we came across another GitHub Repository that has datasets for infants' emotions. 

[Medium.com](https://medium.com/@rtsrumi07/deep-learning-for-classifying-audio-of-infant-crying-with-hands-on-example-a01d3cbf0f74) article by Rabeya Tus Sadia helped us find the dataset [Donate-A-Cry Corpus](https://github.com/gveres/donateacry-corpus) Dataset. Although using this dataset helped us get a model, it wasn't very accurate. While testing, the model would only recognize inputs as 'hungry'. 

We found another dataset that uses [Donate-A-Cry Corpus](https://github.com/gveres/donateacry-corpus) as a skeleton and added more emotions and synthetic data. [BabyCry](https://github.com/martha92/babycry?utm_source=chatgpt.com) is the final dataset we used to create our model.

As stated in [BabyCry](https://github.com/martha92/babycry?utm_source=chatgpt.com): 

- _audio files contain baby cry samples of infants from 0 to 2 years old_
- _corressponding tagging information (the suspected reasons of cry) encoded in the file names_
- _converted all files to WAV file format so that it could be easily read and interconverted by Python audio libraries (librosa, Wave and SoundFile)_

> Data Augmentation Our objective was to balance all the 9 classes, by creating new synthetic training samples by adding small perturbations on our initial training set, so that the model is not biased towards any one single class thus enhance its ability to generalize.

We created two models with two datasets:
1. This dataset uses ONLY the Augmented Data found in the [BabyCry](https://github.com/martha92/babycry?utm_source=chatgpt.com)  repository
2. This dataset uses a combination of the Augmented Data from [BabyCry](https://github.com/martha92/babycry?utm_source=chatgpt.com) and the actual data from [Donate-A-Cry Corpus](https://github.com/gveres/donateacry-corpus)

Each dataset gave us two models in .keras and .h5 file formats, with varying results, which will be discussed later. 

### Preprocess Audio Data
To preprocess the audio data, we extracted the numerical features from the _.wav_ files using librosa. 
```py
# AUDIO PREPROCESSING
def extract_features(file_path):
    y, sr = librosa.load(file_path, sr = SAMPLE_RATE, duration=DURATION)
    mfcc = librosa.feature.mfcc(y=y, sr=sr, n_mfcc=N_MFCC)
    return np.mean(mfcc.T, axis=0)
```
We were able to extract the waveform, spectrogram and MFCC features. 

![belly_pain_features](https://github.com/user-attachments/assets/c5d638d1-c234-497c-8c95-9570c46adbf2)
*Audio Features for Belly Pain*

### Build, Compile, Train, Save
We designed a Convolutional Neural Network (CNN) since it works well with spectrograms. We specified an optimizer as 'adam' and loss function as 'categorical_crossentropy'. The model was saved as .keras and .h5 file. 
```py
# BUILD MODEL 
model = models.Sequential([
    layers.Input(shape=(N_MFCC,)),
    layers.Dense(128, activation='relu'),
    layers.Dropout(0.3),
    layers.Dense(64, activation='relu'),
    layers.Dropout(0.3),
    layers.Dense(len(EMOTIONS), activation='softmax')
])

model.compile(optimizer='adam', loss='categorical_crossentropy', metrics=['accuracy'])

# TRAIN THE MODEL 
history = model.fit(X_train, y_train, validation_data=(X_test, y_test), epochs=760, batch_size=16)  # epochs and batch_size can affect model loss and accuracy rates
                                                                                                    # for augmented model use: epochs = 375 , batch_size = 16
                                                                                                    # for expanded model use: epochs = 760, batch_size = 16 (larger dataset)

# SAVE THE MODEL
model.save(MODEL_PATH+"\\expanded\\baby_emotions_model.keras")          # save model as a .keras for Keras v3 
model.save(MODEL_PATH+"\\expanded\\baby_emotions_model.h5")             # legacy file format (in case of downgrading)
model.save_weights(MODEL_PATH+"\\expanded\\baby_emotions.weights.h5")   # save legacy weights
```
### Evaluation and Model Testing
To evaluate the model's loss, accuracy and other statistics we used the following code: 
```py
# EVALUATE MODEL BASED ON TEST DATASET 
loss, accuracy = model.evaluate(X_test,y_test)
print(f"Test Loss: {loss}")             # display test loss 
print(f"Test Accuracy: {accuracy}")     # display test accuracy

y_pred = model.predict(X_test)
y_pred_classes = np.argmax(y_pred, axis=1)
y_test_classes = np.argmax(y_test, axis=1)

print(classification_report(y_test_classes, y_pred_classes, target_names=EMOTIONS)) # generate classification report 
```
Using the testingModel.py program, we passed through various _.wav_ files with different emotions to see what the model would predict. 

#### Results for Augmented Dataset Model
Test Loss: 0.4573
Test Accuracy: 81.081%

|     | precision | recall | f1-score | support |
| -------- | ------- | -------- | ------- | ------- |
| belly_pain  | 0.83 | 0.62 | 0.71 | 8 |
| burping | 1.00| 1.00 | 1.00 | 6 |
| discomfort | 0.75 | 0.43 | 0.55 | 7 |
| hungry | 1.00 | 1.00 | 1.00 | 7 |
| tired | 0.64 | 1.00 | 0.78 | 9 |
| accuracy |  |  | 0.81 | 37 |
| macro avg | 0.85 | 0.81 | 0.81 | 37 |
| weighted avg | 0.83 | 0.81 | 0.80 | 37 |

![augment_model_test](https://github.com/user-attachments/assets/d8796496-f5d0-4ae2-8004-c3f890b52a44)

*Augmented Model Testing in testingModel.py*

#### Results for Expanded Dataset Model 
Test Loss: 1.2951
Test Accuracy: 71.929%

|     | precision | recall | f1-score | support |
| -------- | ------- | -------- | ------- | ------- |
| belly_pain  | 0.75 | 0.90 | 0.82 | 10 |
| burping | 0.86 | 0.86 | 0.86 | 7 |
| discomfort | 0.70 | 0.58 | 0.64 | 12 |
| hungry | 0.53 | 1.00 | 0.69 | 10 |
| tired | 1.00 | 0.50 | 0.67 | 18 |
| accuracy |  |  | 0.72 | 57 |
| macro avg | 0.85 | 0.77 | 0.73 | 57 |
| weighted avg | 0.83 | 0.72 | 0.71 | 57 |

![expanded_model_test](https://github.com/user-attachments/assets/59a95087-0045-4848-9c59-f0e51eb25bd5)

*Expanded Model Testing in testingModel.py*

## C++ Audio Processing / Recognition

## Compile and Run
### Dependencies 
Use sudo commands to install the following dependencies or run the bash script

Required Dependencies:
```sh
gh / git 
cmake
make
gcc / g++
libopencv-dev
libportaudio2 libportaudio-dev
portaudio19-dev
libsndfile1-dev
libaubio-dev
```

To run the bash script: 
```sh
$ chmod +x install_libs.sh
$ ./install_libs.sh
```
### Running the Project
To run the C++ project, run the Makefile
```sh
$ make
```

