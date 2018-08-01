#include "pyclassification.h"

PyClassification::PyClassification()
{
    PyInit();
}

void PyClassification::PyInit()
{
    Py_Initialize();

    PyObject* pModule = NULL;
    PyObject* pFunc = NULL;
    //PyObject* pParam = NULL;
    PyObject* pNDArray = NULL;
    PyObject* pResult = NULL;
    const char* pBuffer = NULL;
    int iBufferSize = 0;

    qDebug() << "Python Initialized";

    string chdir_cmd = string("sys.path.append(\'/home/zichun/pylon_cv/python\')");
    const char* cstr_cmd = chdir_cmd.c_str();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(cstr_cmd);

    pModule = PyImport_ImportModule("mytest");

    if (!pModule)
    {
        qDebug() << "get module failed!";
        //exit (0);
    }

    pFunc = PyObject_GetAttrString(pModule, "testa");

    if (!pFunc)
    {
        qDebug() << "get func failed!";
        //exit (0);
    }

    cv::Mat img = cv::imread("../images/test.jpg", 1);

    pNDArray = pycvt::fromMatToNDArray(img);

    pResult = PyObject_CallFunctionObjArgs(pFunc, pNDArray, NULL);

    /* This method is to pass non-object para
    pParam = Py_BuildValue("(s)", "MSG from QT");
    pResult = PyEval_CallObject(pFunc, pParam);
    */

    if(pResult)
    {
        if(PyArg_Parse(pResult, "(si)", &pBuffer, &iBufferSize))
        {
            qDebug() << pBuffer;
            qDebug() << iBufferSize;
        }
    }

    Py_DECREF(pModule);
    //Py_DECREF(pParam);
    Py_DECREF(pFunc);
    Py_DECREF(pNDArray);
    Py_DECREF(pResult);

    Py_Finalize();
}
