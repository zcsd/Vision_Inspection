#ifndef PYCLASSIFICATION_H
#define PYCLASSIFICATION_H

#include <QDebug>
#include <QString>

#pragma push_macro("slots")
#undef slots
#include "Python.h"
#pragma pop_macro("slots")

#include <iostream>
using namespace std;

class PyClassification
{
public:
    PyClassification();

private:
    void PyInit();

};

#endif // PYCLASSIFICATION_H
