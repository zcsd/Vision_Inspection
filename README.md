# **Vision Inspection System V1.0**

## **Requirements**
- Basler USB3.0 VISION Camera (Hardware)
- PC with USB3.0 port (Hardware)
- Ubuntu 18.04
- Qt 5.12.0
- OpenCV-4.0.0 or newer
- Pylon 5.1.0 SDK

## **IDE**
- Qt Creator

## **Installation:**

- Install OpenCV-4.0.0(or newer) from native building

```
sudo apt-get install build-essential cmake unzip wget pkg-config
sudo apt-get install libjpeg-dev libpng-dev libtiff-dev
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt-get install libxvidcore-dev libx264-dev
sudo apt-get install libgtk-3-dev
sudo apt-get install libatlas-base-dev gfortran
sudo apt-get install python3-dev
sudo apt install x264 libx264-dev ffmpeg
sudo apt-get install v4l-utils libtbb-dev
sudo apt-get install libvorbis-dev libxvidcore-dev

cd $HOME
wget -O opencv.zip https://github.com/Itseez/opencv/archive/4.0.1.zip
unzip opencv.zip

wget -O opencv_contrib.zip https://github.com/Itseez/opencv_contrib/archive/4.0.1.zip
unzip opencv_contrib.zip

cd opencv
mkdir build
cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D BUILD_opencv_python2=OFF -D BUILD_opencv_python3=OFF -D INSTALL_PYTHON_EXAMPLES=OFF -DBUILD_opencv_java=OFF -D INSTALL_C_EXAMPLES=ON -D WITH_OPENGL=ON -D OPENCV_EXTRA_MODULES_PATH=$HOME/opencv_contrib/modules -D -BUILD_EXAMPLES=ON ..

make -j6
sudo make install
sudo ldconfig
```

- Install Pylon 5 SDK

```
The SDK installation file is in (GTS SHARE FOLDER/Software/Vision/Pylon/pylon_5.1.0.12682-deb0_amd64.deb).
Copy it to local disk, run "chmod +x pylon_5.1.0.12682-deb0_amd64.deb", make it excutable.
Double click the deb file, follow the guide to install by default.
```

- Install Qt (5.12.0 or newer)

```
Install Open Source (GPL v3) version, download it from https://www.qt.io/download, it's ok to use online installer, must tick "Source Code" and "Qt Creator" option in installation window.
It may need longer time to install.
```

- Install Qt Extra Module (5.12.0 or newer)
  Mqtt and OpcUA modules(available in GPL v3) are not included in open source Qt version, we need to compile the module by ourselves if we need to to use it, they are quite convenient to use.
```
qtmqtt:

cd ~/Qt/5.12.0/Src   (your Qt path may be different)
wget -O qtmqtt.zip https://github.com/qt/qtmqtt/archive/v5.12.0.zip
unzip  qtmqtt.zip

cd qtmqtt

mkdir .git (very important)

mkdir build && cd build

~/Qt/5.12.0/gcc_64/bin/qmake -r ..  (your qmake path may be different)

make install

###Finish to install qtmqtt

qtopcua(open62541 as backend):

cd ~/Qt/5.12.0/Src
git clone https://code.qt.io/qt/qtopcua.git
cd qtopcua
git checkout 5.12

mkdir build && cd build

~/Qt/5.12.0/gcc_64/bin/qmake ..

make
sudo make install

###Finish to install qtopcua

```
- For RFID using, serial port ttyUSBx only can be accessed by root by default, you need to add your username to dialout group, and reboot.
```
Put pylon_cv project folder to $HOME folder, can download it from gitlab for gts share folder.
sudo cp ~/pylon_cv/lib/rfid_s8/libs8.so /lib/
sudo usermod -a -G dialout $USER
sudo reboot
```
- sudo apt-get install python3-numpy  (not same as "pip3 install numpy")

- sudo apt install libcurl4-openssl-dev

- sudo apt install python3-pip

- pip3 install opencv-python==3.1.0.4 imutils numpy tensorflow keras


## **Project Setting:**

- Open Qt Creator, click "Open Project" and select "Pylon_CV.pro" in the project folder.
- Use default compiler, wait configuration to be finished.
- Click "Projects" in left panel of Qt Creator, change "Build directory" to $HOME/pylon_cv/build, this setting is very important, related to path setting in code.
- Then build and run the program.
- NOTE: It need longer time for first time using, program need to download model from internet.
You may need to change .pro setting file for some library upgrade.
  
*15 Jan 2019*
