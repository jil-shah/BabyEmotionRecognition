import os
import shutil
import glob
import symptomPrediction 

OUTPUT_DIR = os.getcwd() + "\\output\\"
filename = "mfcc_features.txt"

print("DIR: "+str(OUTPUT_DIR))

while(True):

    inputFile = glob.glob(f"{OUTPUT_DIR}/*{filename}*")[0] # Search with pattern matching
    if inputFile:
        print(f"File(s) found: {inputFile}")
        emotion = symptomPrediction.predictFromFile(inputFile)
        print("emotion: "+emotion)
        os.remove(inputFile)

        cppInput = open(OUTPUT_DIR+"predicted_symptom.txt","w")
        cppInput.write(emotion)
        cppInput.close()

        break

    else:
        print(f"File '{filename}' not found")
        break


