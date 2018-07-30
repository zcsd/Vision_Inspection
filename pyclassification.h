#ifndef PYCLASSIFICATION_H
#define PYCLASSIFICATION_H

#include <QDebug>
#include <QString>

#pragma push_macro("slots")
#undef slots
#include "Python.h"
#pragma pop_macro("slots")

#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/core/core.hpp"

using namespace cv;
using namespace std;

class PyClassification
{
public:
    PyClassification();

private:
    void PyInit();

};

#endif // PYCLASSIFICATION_H
