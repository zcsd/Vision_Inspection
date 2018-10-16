For using libs8.so

copy libs8.so to /lib

sudo cp libs8.so /lib/

For user access serial port ttyUSBx:

sudo usermod -a -G dialout $USER
