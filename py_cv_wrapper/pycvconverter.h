// This wrapper is used to convert python numpy array <==> c++ Mat
// Editor: @ZC  edited from opencv/modules/python/src2/cv2.cpp
// Date: created on 31 Jul 2018
#ifndef PYCVCONVERTER_H
#define PYCVCONVERTER_H

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/ndarrayobject.h>
#include <opencv2/core/core.hpp>
#include <cstdio>

namespace pycvt{

using namespace cv;
static PyObject* opencv_error = 0;

#define ERRWRAP2(expr) \
try \
{ \
    PyAllowThreads allowThreads; \
    expr; \
} \
catch (const cv::Exception &e) \
{ \
    PyErr_SetString(opencv_error, e.what()); \
    return 0; \
}

class PyAllowThreads;

PyObject* fromMatToNDArray(const Mat& m);
Mat fromNDArrayToMat(PyObject* o);

}

#endif   // PYCVCONVERTER_H
