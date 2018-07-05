// FDShapeMatching class
// Author: @ZC
// Date: created on 04 Jul 2018
#ifndef FOURIERDESCRIPTOR_H
#define FOURIERDESCRIPTOR_H

#include <opencv2/core.hpp>
#include <opencv2/core/ocl.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/cvdef.h>
#include <opencv2/core/core_c.h>
#include <opencv2/imgproc.hpp>

#include <map>
#include <math.h>
#include <vector>
#include <iostream>

using namespace cv;

class FDShapeMatching
{

public:
    FDShapeMatching(int ctr=1024,int fd=16);
    void estimateTransformation(InputArray src, InputArray dst, OutputArray alphaPhiST, double *dist = 0, bool fdContour = false);
    void setCtrSize(int n);
    void setFDSize(int n);
    int getCtrSize();
    int getFDSize();

private:
    int ctrSize;
    int fdSize;
    std::vector<std::complex<double> > b;
    std::vector<std::complex<double> > a;
    std::vector<double> frequence;
    std::vector<double> rho, psi;
    void frequencyInit();
    void fAlpha(double x, double &fn, double &df);
    double distance(std::complex<double> r, double alpha);
    double newtonRaphson(double x1, double x2);
};

void fourierDescriptor(InputArray src, OutputArray dst, int nbElt=-1,int nbFD=-1);
void transformFD(InputArray src, InputArray t,OutputArray dst, bool fdContour=true);
void contourSampling(InputArray src, OutputArray out, int nbElt);

#endif // FOURIERDESCRIPTOR_H
