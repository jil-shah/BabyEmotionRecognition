#!/bin/bash

# Open terminal for Python script in virtual environment
lxterminal --working-directory=/home/raspberry/BabyEmotionRecognition/python --command="bash -c 'source /home/raspberry/tf-venv/bin/activate && python main.py; exec bash'"

# Wait 3 seconds before starting Python
sleep 3

# Open terminal for C++ program
lxterminal --working-directory=/home/raspberry/BabyEmotionRecognition --command="bash -c 'make && ./BabyBeacon; exec bash'"
