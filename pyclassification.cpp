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
    PyObject* pParam = NULL;
    PyObject* pResult = NULL;
    const char* pBuffer = NULL;
    qDebug() << "Python Initialized";

    int iBufferSize = 0;

    string chdir_cmd = string("sys.path.append(\'/home/zichun/pylon_cv\')");
    const char* cstr_cmd = chdir_cmd.c_str();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(cstr_cmd);

    pModule = PyImport_ImportModule("mytest");

    if (!pModule)
    {
        qDebug() << "get module failed!";
        //exit (0);
    }

    pFunc = PyObject_GetAttrString(pModule, "main");

    if (!pFunc)
    {
        qDebug() << "get func failed!";
        //exit (0);
    }

    //PyObject_CallFunction(pFunc,NULL);

    pParam = Py_BuildValue("(s)", "HEHEHE");
    pResult = PyEval_CallObject(pFunc,pParam);
    if(pResult)
    {
        if(PyArg_Parse(pResult, "(si)", &pBuffer, &iBufferSize))
        {
            qDebug() << pBuffer;
            qDebug() << iBufferSize;
        }
    }
    Py_DECREF(pParam);
    Py_DECREF(pFunc);

    Py_Finalize();
}
