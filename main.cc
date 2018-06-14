// Vision Software using Pylon SDK and OpenCV3.4 lib
// Author: @zichun-SAT
// Date: created on 12 Jun 2018
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include "frame_grabber.h"

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
    
    FrameGrabber frameGrabber;

    if (!frameGrabber.getCameraStatus()) {
        cout << "[ERROR!] Fail to Open Camera!" << endl;
        return 0;
    }

    cv::namedWindow("Display", CV_WINDOW_NORMAL);
    cv::Mat originalFrame, displayedFrame;

    while (frameGrabber.getCurrentFrame(originalFrame)) {
        cv::resize(originalFrame, displayedFrame, Size(), 0.5, 0.5, INTER_LINEAR);
        cv::imshow("Display", displayedFrame);

        if ( (cv::waitKey(1) & 0xFF) == 'q' ) break;
    }
    
    return 0;
}
