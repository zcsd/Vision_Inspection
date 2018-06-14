#include "frame_grabber.h"

FrameGrabber::FrameGrabber()
{
    Pylon::PylonInitialize();

    cameraStatus = false;
    pylonCamera = new Camera_t;

    checkCameraStatus();
    configureCamera();
    startGrabbing();   
}

void FrameGrabber::checkCameraStatus()
{
    // Get all the connected camera
    Pylon::DeviceInfoList_t cameraList;
    Pylon::CTlFactory::GetInstance().EnumerateDevices(cameraList, true);

    if (cameraList.size() == 0) {
        cout << "[WARNING!] No Camera Detected, Please Check Connection!" << endl;
        cameraStatus = false;
    }
    else {
        // Show all the connected cameras name
        for (int i=0; i < cameraList.size(); i++)
            cout << "[INFO] "<< cameraList[i].GetFriendlyName() << " Camera Connected." << endl;
        try {
            // Attach connected camera to the pylon camera object
            pylonCamera->Attach(CTlFactory::GetInstance().CreateFirstDevice());
            // Open the camera, but not yet start grabbing
            pylonCamera->Open();
            // Warm up camera for 0.5s
            usleep(500000);
            cameraStatus = true;
        }
        catch (GenICam::GenericException &e) {
        // Error handling.
        cerr << "[ERROR!] An Exception Occurred in cameraStatus()."
        << endl << e.GetDescription() << endl;
        }
    }
}

bool FrameGrabber::getCameraStatus()
{
    return cameraStatus;
}

void FrameGrabber::configureCamera()
{
    if (pylonCamera->IsGrabbing()) pylonCamera->StopGrabbing();

    try {
        // Method 1:
        // Load pfs file into the camera's node map with enabled validation.
        //const char cameraParaFile[] = "config/acA2040-55uc_22095198.pfs";
        //CFeaturePersistence::Save(cameraParaFile, &pylonCamera->GetNodeMap());
        //CFeaturePersistence::Load(cameraParaFile, &pylonCamera->GetNodeMap(), true);

        // Method 2:
        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        
        //pylonCamera->MaxNumBuffer = 10;

        pylonCamera->ExposureAuto.SetValue(ExposureAuto_Off);
        pylonCamera->ExposureMode.SetValue(ExposureMode_Timed);
        pylonCamera->ExposureTime.SetValue(15000); // us

        pylonCamera->AcquisitionFrameRateEnable.SetValue(true);
        pylonCamera->AcquisitionFrameRate.SetValue(30.0);

        pylonCamera->Width.SetValue(2048);
        pylonCamera->Height.SetValue(1536);

        //pylonCamera.CenterX.SetValue(true);
        //pylonCamera.CenterY.SetValue(true);
        // Warm up camera for 0.2s
        usleep(200000);
    }
    catch (GenICam::GenericException &e) {
        // Error handling.
        cerr << "[ERROR!] An Exception Occurred in configureCamera()."
        << endl << e.GetDescription() << endl;
    }
}

void FrameGrabber::startGrabbing()
{
    try {
        pylonCamera->StartGrabbing(GrabStrategy_LatestImageOnly);
        cout << "[INFO] Camera Start Grabbing Frame." << endl;
    }
    catch (GenICam::GenericException &e) {
        // Error handling.
        cerr << "[ERROR!] An Exception Occurred in startGrabbing()."
        << endl << e.GetDescription() << endl;
    }
}

bool FrameGrabber::getCurrentFrame(cv::Mat &cvFrame)
{
    try {
        CPylonImage pylonFrame;
        CImageFormatConverter formatConverter;
        formatConverter.OutputPixelFormat = PixelType_BGR8packed;
        // This smart pointer will receive the grab result data.
        GrabResultPtr_t ptrGrabResult;
        // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
        pylonCamera->RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

        if (ptrGrabResult->GrabSucceeded()) {
            // Convert the grabbed buffer to pylon image
            formatConverter.Convert(pylonFrame, ptrGrabResult);
            // Create an OpenCV image out of pylon image
            cv::Mat tempFrame = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(),
                                        CV_8UC3, (uint8_t *) pylonFrame.GetBuffer());
            cvFrame = tempFrame.clone();
            return 1;
        }
        else {
            return 0;
        }
    }
    catch (GenICam::GenericException &e) {
        // Error handling.
        cerr << "[ERROR!] An Exception Occurred in getCurrentFrame()."
        << endl << e.GetDescription() << endl;
        return 0;
    }
}

FrameGrabber::~FrameGrabber()
{
    pylonCamera->StopGrabbing();
    pylonCamera->Close();
    Pylon::PylonTerminate();
    cout << "[INFO] Camera Released Safely." << endl;
}
