import os
import shutil
import glob
import symptomPrediction 

OUTPUT_DIR = symptomPrediction.DIR + "/output/"
filename = "mfcc_features.txt"

print("DIR: "+str(OUTPUT_DIR))

flag = 1

while(True):
        
    inputFile = glob.glob(f"{OUTPUT_DIR}/*{filename}*") # Search with pattern matching
    if flag == 1:
        print("Waiting for input file....")
        flag = 0 
        
    if inputFile:
        print(f"File(s) found: {inputFile[0]}")
        emotion = symptomPrediction.predictFromFile(inputFile[0])
        print("emotion: "+emotion)
        os.remove(inputFile[0])

        cppInput = open(OUTPUT_DIR+"predicted_symptom.txt","w")
        cppInput.write(emotion)
        cppInput.close()
        flag = 1
    else:
        flag = 0
        continue
        


