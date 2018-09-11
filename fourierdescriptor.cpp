#include "fourierdescriptor.h"

FDShapeMatching::FDShapeMatching(int ctr,int fd) : ctrSize(ctr),fdSize(fd)
{

}

FDShapeMatching::~FDShapeMatching()
{

}

void FDShapeMatching::setCtrSize(int n)
{
    ctrSize = n;
}

void FDShapeMatching::setFDSize(int n)
{
    fdSize = n;
}

int FDShapeMatching::getCtrSize()
{
    return ctrSize;
}

int FDShapeMatching::getFDSize()
{
    return fdSize;
}

void FDShapeMatching::frequencyInit()
{
    int nbElt = ctrSize;
    frequence.resize(ctrSize);

    for (int i = 0; i <= nbElt / 2; i++)
        frequence[i] = 2 * M_PI*(float)i / nbElt;
    for (int i = nbElt / 2 + 1; i<nbElt; i++)
        frequence[i] = 2 * M_PI*(float)(i - nbElt) / nbElt;
}

void FDShapeMatching::fAlpha(double x, double &fn, double &df)
{
    int nbElt = static_cast<int>(rho.size());
    double s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    double ds1 = 0, ds2 = 0, ds3 = 0, ds4 = 0;

    for (int n = 1; n <= fdSize; n++)
    {
        s1 += rho[n] * sin(psi[n] + frequence[n] * x) +
            rho[nbElt - n] * sin(psi[nbElt - n] + frequence[nbElt - n] * x);
        s2 += frequence[n] * rho[n] * cos(psi[n] + frequence[n] * x) +
            frequence[nbElt - n] * rho[nbElt - n] * cos(psi[nbElt - n] + frequence[nbElt - n] * x);
        s3 += rho[n] * cos(psi[n] + frequence[n] * x) +
            rho[nbElt - n] * cos(psi[nbElt - n] + frequence[nbElt - n] * x);
        s4 += frequence[n] * rho[n] * sin(psi[n] + frequence[n] * x) +
            frequence[nbElt - n] * rho[nbElt - n] * sin(psi[nbElt - n] + frequence[nbElt - n] * x);
        ds1 += frequence[n] * rho[n] * cos(psi[n] + frequence[n] * x) +
            frequence[nbElt - n] * rho[nbElt - n] * cos(psi[nbElt - n] + frequence[nbElt - n] * x);
        ds2 += -frequence[n] * frequence[n] * rho[n] * sin(psi[n] + frequence[n] * x) -
            frequence[nbElt - n] * frequence[nbElt - n] * rho[nbElt - n] * sin(psi[nbElt - n] + frequence[nbElt - n] * x);
        ds3 += -frequence[n] * rho[n] * sin(psi[n] + frequence[n] * x) -
            frequence[nbElt - n] * rho[nbElt - n] * sin(psi[nbElt - n] + frequence[nbElt - n] * x);
        ds4 += frequence[n] * frequence[n] * rho[n] * cos(psi[n] + frequence[n] * x) +
            frequence[nbElt - n] * frequence[nbElt - n] * rho[nbElt - n] * cos(psi[nbElt - n] + frequence[nbElt - n] * x);
    }
    fn = s1 * s2 - s3 *s4;
    df = ds1 * s2 + s1 * ds2 - ds3 * s4 - s3 * ds4;
}

double FDShapeMatching::distance(std::complex<double> r, double alpha)
{
    std::complex<double> j(0, 1);
    double d = 0;

    for (int i = 1; i <= fdSize; i++)
        d += abs(a[i] - b[i] * r*exp(j*alpha*frequence[i])) + abs(a[a.size() - i] - b[a.size() - i] * r*exp(j*alpha*frequence[a.size() - i]));
    return d/fdSize/2;
}

double FDShapeMatching::newtonRaphson(double x1, double x2)
{
    double f1,df1;
    fAlpha(x1, f1, df1);
    if (f1 < 0)
    {
        x1=x2;
        fAlpha(x1, f1, df1);
    }
    CV_Assert(f1 >= 0);
    if (f1 == 0)
        return x1;
    for (int i = 0; i < 5; i++)
    {
        x1 = x1 -f1/df1;
        fAlpha(x1, f1, df1);
        if (f1 == 0)
            return x1;
    }
    return x1;
}

void FDShapeMatching::estimateTransformation(Mat _srcFD, Mat _refFD, Mat *_alphaPhiST, double *distFin)
{
    CV_Assert( fdSize <= ctrSize / 2 - 1);
    if (_srcFD.type() != CV_64FC2)
        _srcFD.convertTo(_srcFD, CV_64F);
    if (_refFD.type() != CV_64FC2)
        _refFD.convertTo(_refFD, CV_64F);

    rho.resize(ctrSize);
    psi.resize(ctrSize);
    b.resize(ctrSize);
    a.resize(ctrSize);
    frequencyInit();
    double alphaMin, phiMin, sMin;
    long n, nbElt = ctrSize;
    double s1, s2, sign1, sign2, df, x1 = nbElt, x2 = nbElt, dx;
    double dist, distMin = 10000, alpha, s, phi;
    std::complex<double> j(0, 1), zz;

    for (n = 0; n<nbElt; n++)
    {
        b[n] = std::complex<double>(_srcFD.at<Vec2d>(n,0)[0], _srcFD.at<Vec2d>(n, 0)[1]);
        a[n] = std::complex<double>(_refFD.at<Vec2d>(n, 0)[0], _refFD.at<Vec2d>(n, 0)[1]);
        zz = conj(a[n])*b[n];
        rho[n] = abs(zz);
        psi[n] = arg(zz);
    }

    x1 = nbElt, x2 = nbElt;
    sMin = 1;
    alphaMin = 0;
    phiMin = arg(a[1] / b[1]);
    do
    {
        x2 = x1;
        fAlpha(x2, sign2, df);
        dx = 1;
        x1 = x2;
        do
        {
            x2 = x1;
            x1 -= dx;
            fAlpha(x1, sign1, df);
        }
        while ((sign1*sign2>0) && (x1>-nbElt));
        if (sign1*sign2<0)
        {
            alpha=newtonRaphson(x1, x2);
            s1 = 0;
            s2 = 0;
            for (n = 1; n<nbElt; n++)
            {
                s1 += rho[n] * sin(psi[n] + frequence[n] * alpha);
                s2 += rho[n] * cos(psi[n] + frequence[n] * alpha);
            }
            phi = -atan2(s1, s2);
            s1 = 0;
            s2 = 0;
            for (n = 1; n < nbElt; n++)
            {
                s1 += rho[n] * cos(psi[n] + frequence[n] * alpha + phi);
                s2 += abs(b[n] * conj(b[n]));
            }
            s = s1 / s2;
            zz = s*exp(j * phi);
            if (s>0)
                dist = distance(zz, alpha);
            else
                dist = 10000;
            if (dist<distMin)
            {
                distMin = dist;
                alphaMin = alpha;
                phiMin = phi;
                sMin = s;
            }
        }
    }

    while ((x1>-nbElt));
    Mat x=(Mat_<double>(1,5)<<alphaMin/ nbElt,phiMin,sMin, _refFD.at<Vec2d>(0, 0)[0]- _srcFD.at<Vec2d>(0, 0)[0], _refFD.at<Vec2d>(0, 0)[1]- _srcFD.at<Vec2d>(0, 0)[1]);
    if (distFin)
        *distFin = distMin;
    x.copyTo(*_alphaPhiST);
}

void fourierDescriptor(InputArray _src, OutputArray _dst, int nbElt, int nbFD)
{
    Mat z = _src.getMat();
    if (nbElt==-1)
        nbElt = getOptimalDFTSize(max(z.rows, z.cols));
    Mat  Z;
    if (z.rows*z.cols!=nbElt)
        contourSampling(_src, z,nbElt);
    else if (_src.depth() == CV_32S)
        z.convertTo(z, CV_32F);
    dft(z, Z, DFT_SCALE | DFT_REAL_OUTPUT);
    if (nbFD == -1)
    {
        Z.copyTo(_dst);
    }
    else
    {
        int n1 = nbFD / 2, n2 = nbElt - n1;
        Mat d(nbFD, 1, Z.type());
        Z.rowRange(Range(1, n1+1)).copyTo(d.rowRange(Range(0, n1)));
        if (n2 > 0)
            Z.rowRange(Range(n2, Z.rows)).copyTo(d.rowRange(Range(n1, nbFD)));
        d.copyTo(_dst);
    }
}

void contourSampling(InputArray _src, OutputArray _out, int nbElt)
{
    Mat ctr;
    _src.getMat().convertTo(ctr,CV_32F);

    double l1 = 0, l2, p, d, s;
    Mat r;
    if (ctr.rows==1)
        ctr=ctr.t();
    int j = 0;
    int nb = ctr.rows;
    p = arcLength(ctr, true);
    l2 = norm(ctr.row(j) - ctr.row(j + 1)) / p;
    for (int i = 0; i<nbElt; i++)
    {
        s = (float)i / (float)nbElt;
        while (s >= l2)
        {
            j++;
            l1 = l2;
            d = norm(ctr.row(j % nb) - ctr.row((j + 1) % nb));
            l2 = l1 + d / p;
        }
        if ((s >= l1) && (s < l2))
        {
            Mat d1=ctr.row((j + 1) % nb);
            Mat d0=ctr.row(j % nb);
            Mat d10 = d1 - d0;
            Mat pn = d0 + d10 * (s - l1) / (l2 - l1);
            r.push_back(pn);
        }
    }
    r.copyTo(_out);
}

void transformFD(InputArray _src, InputArray _t,OutputArray _dst,  bool fdContour)
{
    if (!fdContour)
        CV_Assert(_src.kind() == _InputArray::STD_VECTOR || _src.kind() == _InputArray::MAT);
    else
        CV_Assert( _src.kind() == _InputArray::MAT );
    CV_Assert(_src.channels() == 2);
    CV_Assert(_t.kind() == _InputArray::MAT);
    Mat t=_t.getMat();
    CV_Assert(t.rows == 1 && t.cols==5 && t.depth()==CV_64F);
    Mat Z;
    if (!fdContour)
    {
        Mat ctr1 = _src.getMat();
        if (ctr1.rows==1)
            ctr1 = ctr1.t();
        Mat newCtr1;
        int M = getOptimalDFTSize(ctr1.rows);
        contourSampling(ctr1, newCtr1, M);
        fourierDescriptor(newCtr1, Z);
    }
    else
        Z = _src.getMat();
    if (Z.type()!=CV_64FC2)
        Z.convertTo(Z,CV_64F);
    std::complex<double> expitheta = t.at<double>(0,2)  * std::complex<double>(cos(t.at<double>(0, 1)), sin(t.at<double>(0, 1)));
    for (int j = 1; j<Z.rows; j++)
    {
        std::complex<double> zr(Z.at<Vec2d>(j, 0)[0], Z.at<Vec2d>(j,0)[1]);
        if (j<=Z.rows/2)
            zr = zr*expitheta*exp(t.at<double>(0, 0) * 2 * (M_PI*j) *  std::complex<double>(0, 1));
        else
            zr = zr*expitheta*exp(t.at<double>(0, 0)* 2 * (M_PI*(j - Z.rows))    * std::complex<double>(0, 1));
        Z.at<Vec2d>(j, 0) = Vec2d(zr.real(),zr.imag());
    }
    Z.at<Vec2d>(0, 0) += Vec2d(t.at<double>(0, 3), t.at<double>(0, 4));
    std::vector<Point2d> z;
    dft(Z, z, DFT_INVERSE);
    Mat(z).copyTo(_dst);
}
