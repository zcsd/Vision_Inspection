# **Vision Inspection System**

## **Installation**

**Requirements**
- Basler USB3.0 VISION Camera
- PC with USB3.0 port
- Ubuntu(16.04+) (could extend to support Windows or other Linux)
- Qt 5.12 (LTS)
- OpenCV-3.4.3
- Pylon 5.10 SDK

**Recommend IDE**
- QT Creator

**Steps:**

- Install OpenCV-3.4.2 from native building

- Install Pylon 5 SDK

- Install QT

- sudo apt-get install python3-numpy

- sudo apt install libcurl4-openssl-dev

- for RFID lib, serial port ttyUSBx only can be accessed by root by default, you need to add your username to dialout group, and reboot.

sudo usermod -a -G dialout $USER



## **TODO**

- Move connect/disconnect/capture/stream to upper menu.


*Jul 12 2018*
