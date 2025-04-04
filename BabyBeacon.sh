#!/bin/bash

# Open terminal for C++ program
gnome-terminal -- bash -c "cd BabyEmotionRecognition && make && ./BabyBeacon; exec bash"

# Open terminal for Python script in virtual environment
gnome-terminal -- bash -c "cd ~ && source ../tf-venv/bin/activate && cd /home/raspberry/BabyEmotionRecognition/python && python script.py; exec bash"
