// FrameGrabber class for Basler USB3.0 camera
// Author: @zichun-SAT
// Date: created on 12 Jun 2018
#ifndef FRAME_GRABBER_H
#define FRAME_GRABBER_H

#include <iostream>
#include <unistd.h> // for usleep()

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
// Namespace for using pylon objects.
using namespace Pylon;
// Settings for using Basler USB cameras.
#include <pylon/usb/BaslerUsbInstantCamera.h>
typedef Pylon::CBaslerUsbInstantCamera Camera_t;
typedef Camera_t::GrabResultPtr_t GrabResultPtr_t;
using namespace Basler_UsbCameraParams;

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace cv;
using namespace std;

class FrameGrabber
{

public:
    FrameGrabber();
    ~FrameGrabber();
    bool getCameraStatus(); // Return camera connection status
    bool getCurrentFrame(cv::Mat &cvFrame); // Catch frame function for public
        
private:
    Camera_t *pylonCamera;
    bool cameraStatus; // Camera connection status
    void checkCameraStatus(); // Check camera connection status
    void configureCamera(); // Set camera parameters
    void startGrabbing();
};

#endif