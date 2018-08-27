#include "framegrabber.h"

FrameGrabber::FrameGrabber(QObject *parent) : QObject(parent)
{
    initCamera();
}

FrameGrabber::~FrameGrabber()
{
    if (pylonCamera->IsOpen()) {
        if (pylonCamera->IsGrabbing())  pylonCamera->StopGrabbing();
        pylonCamera->Close();
        cameraConnected = false;
    }

    delete pylonCamera;
    Pylon::PylonTerminate();
}

void FrameGrabber::initCamera()
{
    Pylon::PylonInitialize();
    pylonCamera = new Camera_t;
    cameraConnected = false;
    startGrabbing = false;
    grabMode = 'N';
}

void FrameGrabber::receiveConnectCamera()
{
    try {
        // Attach connected camera to the pylon camera object
        pylonCamera->Attach(CTlFactory::GetInstance().CreateFirstDevice());
        // Open the camera, but not yet start grabbing
        pylonCamera->Open();
        // Warm up camera for 0.1s or 100ms
        usleep(100000);
        if(pylonCamera->IsOpen()) {
            cameraConnected = true;
            configureCamera();
        }
        else {
            cameraConnected = false;
            qDebug() << "FAIL to Connect Camera.";
        }
    }
    catch (GenICam::GenericException &e) {
        // Error handling.
        cerr << "[ERROR!] An Exception Occurred in connectCamera()."
             << endl << e.GetDescription() << endl;
    }
}

void FrameGrabber::configureCamera()
{
    if (pylonCamera->IsGrabbing()) pylonCamera->StopGrabbing();

    try {
        // Method 1:
        // Load pfs file into the camera's node map with enabled validation.
        //const char cameraParaFile[] = "../conf/acA2040-55uc_22095198.pfs";
        const char cameraParaFile[] = "../conf/acA2440-35uc_22776933.pfs";
        //CFeaturePersistence::Save(cameraParaFile, &pylonCamera->GetNodeMap());
        CFeaturePersistence::Load(cameraParaFile, &pylonCamera->GetNodeMap(), true);

        // Method 2:
        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.

        //pylonCamera->MaxNumBuffer = 10;
        /*
        pylonCamera->ExposureAuto.SetValue(ExposureAuto_Off);
        pylonCamera->ExposureMode.SetValue(ExposureMode_Timed);
        pylonCamera->ExposureTime.SetValue(25000); // us

        pylonCamera->AcquisitionFrameRateEnable.SetValue(true);
        pylonCamera->AcquisitionFrameRate.SetValue(35.0);

        pylonCamera->Width.SetValue(2048);
        pylonCamera->Height.SetValue(1536);
        */
        //pylonCamera->CenterX.SetValue(true);
        //pylonCamera->CenterY.SetValue(true);
        // Warm up camera for 0.1s
        usleep(100000);
    }
    catch (GenICam::GenericException &e) {
        // Error handling.
        cerr << "[ERROR!] An Exception Occurred in configureCamera()."
             << endl << e.GetDescription() << endl;
    }
}

void FrameGrabber::receiveDisconnectCamera()
{
    if (pylonCamera->IsOpen()) {
        if (pylonCamera->IsGrabbing())  pylonCamera->StopGrabbing();
        pylonCamera->Close();
        cameraConnected = false;
        grabMode = 'N';
        startGrabbing = false;
    }
    pylonCamera->DetachDevice();
}

void FrameGrabber::receiveStartCaptureMode()
{
    grabMode = 'C';
    if (!startGrabbing) {
        try {
            pylonCamera->StartGrabbing(GrabStrategy_LatestImageOnly);
            startGrabbing = true;
        }
        catch (GenICam::GenericException &e) {
            // Error handling.
            cerr << "[ERROR!] An Exception Occurred in startCapture()."
                 << endl << e.GetDescription() << endl;
        }
    }

    emit sendCaptureFrame(readFrame());
}

void FrameGrabber::receiveStartStreamMode()
{
    grabMode = 'S';
    if (!startGrabbing) {
        try {
            pylonCamera->StartGrabbing(GrabStrategy_LatestImageOnly);
            startGrabbing = true;
        }
        catch (GenICam::GenericException &e) {
            // Error handling.
            cerr << "[ERROR!] An Exception Occurred in startStream()."
                 << endl << e.GetDescription() << endl;
        }
    }
}

void FrameGrabber::receiveSendFrame()
{
    if (startGrabbing) {
        emit sendFrame(readFrame());
    }
}

cv::Mat FrameGrabber::readFrame()
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

            return cvFrame;
        }
        else {
            qDebug() << "Fail to Grab Frame.";
        }
    }
    catch (GenICam::GenericException &e) {
        // Error handling.
        cerr << "[ERROR!] An Exception Occurred in readFrame()."
             << endl << e.GetDescription() << endl;
    }
}

void FrameGrabber::receiveStopGrabbing()
{
    if (startGrabbing) {
        if (pylonCamera->IsGrabbing()) pylonCamera->StopGrabbing();
        startGrabbing = false;
        grabMode = 'N';
    }
}

QString FrameGrabber::scanDevices()
{
    // Get all the connected camera
    Pylon::DeviceInfoList_t cameraList;
    Pylon::CTlFactory::GetInstance().EnumerateDevices(cameraList, true);
    QString cameraNameString = NULL;
    if (cameraList.size() != 0) {
        cameraNameString = QString(cameraList[0].GetFriendlyName());
    }

    return cameraNameString;
}
