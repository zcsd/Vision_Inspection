#!/bin/bash
## This guide contains step by step instructions to install cuda v9.0 and cudnn 7.2 in ubuntu 18.04
## currently(27 Sep 2018), tensorflow-gpu verison only work with cuda 9.0(except native compile by yourself)

### to verify your gpu is cuda enable check
lspci | grep -i nvidia

### check your current gcc verision(default is 7.3 for ubuntu18.04)
gcc --version

# install nvidia driver(at least 384)
# No need to install 384 if your nvidia driver version is newer than 384
sudo add-apt-repository ppa:graphics-drivers/ppa
sudo apt update
sudo apt install nvidia-384 nvidia-384-dev

# install other import packages
sudo apt-get install g++ freeglut3-dev build-essential libx11-dev libxmu-dev libxi-dev libglu1-mesa libglu1-mesa-dev

# CUDA 9.0 requires gcc 5 or 6
sudo apt install gcc-5
sudo apt install g++-5
# set gcc 5 as primary version
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 50
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 50

# Prepare "runfile (local)" installation packages from cuda toolkit archive(already in folder)
# base file: cuda_9.0.176_384.81_linux-run
# patch file1: cuda_9.0.176.1_linux.run
# patch file2: cuda_9.0.176.2_linux.run
# patch file3: cuda_9.0.176.3_linux.run
# patch file4: cuda_9.0.176.4_linux.run

# make the download file executable
chmod +x cuda_9.0.176_384.81_linux-run 
sudo ./cuda_9.0.176_384.81_linux-run --override

# answer following questions while installation begin
# You are attempting to install on an unsupported configuration. Do you wish to continue? y
# Install NVIDIA Accelerated Graphics Driver for Linux-x86_64 384.81? n
# Install the CUDA 9.0 Toolkit? y  y  y 

# install cuBLAS patch file
chmod +x cuda_9.0.176.1_linux.run
sudo ./cuda_9.0.176.1_linux.run
chmod +x cuda_9.0.176.2_linux.run
sudo ./cuda_9.0.176.2_linux.run
chmod +x cuda_9.0.176.3_linux.run
sudo ./cuda_9.0.176.3_linux.run
chmod +x cuda_9.0.176.4_linux.run
sudo ./cuda_9.0.176.4_linux.run

# set up symlinks for gcc/g++
sudo ln -s /usr/bin/gcc /usr/local/cuda/bin/gcc
sudo ln -s /usr/bin/g++ /usr/local/cuda/bin/g++

# setup your paths
echo 'export PATH=/usr/local/cuda-9.0/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda-9.0/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

# you can 
# install cuDNN v7.2
tar -xzvf cudnn-9.0-linux-x64-v7.2.1.38.tgz

# copy the following files into the cuda toolkit directory.
sudo cp -P cuda/include/cudnn.h /usr/local/cuda-9.0/include
sudo cp -P cuda/lib64/libcudnn* /usr/local/cuda-9.0/lib64/
sudo chmod a+r /usr/local/cuda-9.0/lib64/libcudnn*

# finally, to verify the installation, check
nvidia-smi
nvcc -V

# CUDA and cuDNN has been installed

# install tensorflow-gpu
sudo apt install python3-pip
pip3 install tensorflow-gpu
pip3 install keras