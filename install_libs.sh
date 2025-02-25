#!/bin/bash

echo "Updating package list..."
sudo apt update

echo "Installing git dependancies..."
sudo apt install -y gh
sudo apt install -y git

echo "Installing C++ compiler and build tools..."
sudo apt install -y cmake
sudo apt install -y make
sudo apt install -y gcc
sudo apt install -y g++

echo "Installing OpenCV library..."
sudo apt install -y libopencv-dev
pkg-config --modversion opencv4

echo "Installing PortAudio..."
sudo apt install -y libportaudio2 libportaudio-dev
sudo apt install -y portaudio19-dev
sudo apt install -y libsndfile1-dev
sudo apt install -y libaubio-dev
pkg-config --modversion portaudio-2.0
pkg-config --modversion sndfile

echo "Installation Successful"
