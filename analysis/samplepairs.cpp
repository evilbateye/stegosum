#include <QImage>
#include <QColor>
#include <QtCore/qmath.h>
#include <QDebug>

#include "samplepairs.h"

SamplePairs::SamplePairs()
{
}

/**
 * Code inspired by Digital Invisible Ink Toolkit project by Kathryn Hempstalk (http://diit.sourceforge.net/)
 * @brief SamplePairs::analyse
 * @param image
 * @param color
 * @return
 */
void SamplePairs::analyseOld(QImage &image, Color color, bool overlap)
{
    Q_UNUSED(overlap);

    //get the images sizes
    int imgw = image.width();
    if (imgw % 2 == 1) imgw--;

    int imgh = image.height();
    if (imgh % 2 == 1) imgh--;

    QRgb * firstPixel;
    QRgb * secondPixel;
    quint8 u;
    quint8 v;
    long P,X,Y,Z;
    long W;

    P = X = Y = Z = W = 0;

    mMessageLength = 0;

    //pairs across the image
    for (int starty = 0; starty < imgh; starty++) {
        for (int startx = 0; startx < imgw; startx = startx + 2) {
            //get the block of data (2 pixels)
            firstPixel = &reinterpret_cast<QRgb *>(image.scanLine(starty))[startx];
            secondPixel = &reinterpret_cast<QRgb *>(image.scanLine(starty))[startx + 1];

            u = getColor(color, firstPixel);
            v = getColor(color, secondPixel);

            //if the 7 msb are the same, but the 1 lsb are different
            if ((u >> 1 == v >> 1) && ((v & 0x01) != (u & 0x01)))
                W++;
            //if the pixels are the same
            if (u == v)
                Z++;
            //if lsb(v) = 0 & u < v OR lsb(v) = 1 & u > v
            if ((((v & 0x01) == 0) && (u < v)) || (((v & 0x01) == 1) && (u > v)))
                X++;
            //vice versa
            if ((((v & 0x01) == 0) && (u > v)) || (((v & 0x01) == 1) && (u < v)))
                Y++;
            P++;
        }
    }

    //pairs down the image
    for (int starty = 0; starty < imgh; starty = starty + 2) {
        for (int startx = 0; startx < imgw; startx++) {
            //get the block of data (2 pixels)
            firstPixel = &reinterpret_cast<QRgb *>(image.scanLine(starty))[startx];
            secondPixel = &reinterpret_cast<QRgb *>(image.scanLine(starty + 1))[startx];

            u = getColor(color, firstPixel);
            v = getColor(color, secondPixel);

            //if the 7 msb are the same, but the 1 lsb are different
            if ((u >> 1 == v >> 1) && ((v & 0x01) != (u & 0x01)))
                W++;
            //if the pixels are the same
            if (u == v)
                Z++;
            //if lsb(v) = 0 & u < v OR lsb(v) = 1 & u > v
            if ((((v & 0x01) == 0) && (u < v)) || (((v & 0x01) == 1) && (u > v)))
                X++;
            //vice versa
            if ((((v & 0x01) == 0) && (u > v)) || (((v & 0x01) == 1) && (u < v)))
                Y++;
            P++;
        }
    }

    //solve the quadratic equation
    //in the form ax^2 + bx + c = 0
    double a = 0.5 * ( W + Z );
    double b = 2 * X - P;
    double c = Y - X;

    //the result
    double x;

    //straight line
    if (a == 0)
        x = c / b;

    //curve
    //take it as a curve
    double discriminant = qPow(b, 2) - (4 * a * c);

    if (discriminant >= 0) {
        double rootpos = ((-b) + qSqrt(discriminant)) / (2 * a);
        double rootneg = ((-b) - qSqrt(discriminant)) / (2 * a);

        //return the root with the smallest absolute value (as per paper)
        if(qAbs(rootpos) <= qAbs(rootneg))
            x = rootpos;
        else
            x = rootneg;
    } else {
        x = c / b;
    }

    if (x == 0) {
        //let's assume straight lines again, something is probably wrong
        x = c / b;
    }

    mMessageLength = x;
}

void SamplePairs::categorize(int m, int x1,int y1, int x2, int y2, Color color, QImage & image)
{
    QRgb * firstPixel;
    QRgb * secondPixel;

    firstPixel = &reinterpret_cast<QRgb *>(image.scanLine(y1))[x1];
    secondPixel = &reinterpret_cast<QRgb *>(image.scanLine(y2))[x2];

    quint8 u = getColor(color, firstPixel);
    quint8 v = getColor(color, secondPixel);

    int d = qAbs(u - v);
    int cd = qAbs((u >> 1) - (v >> 1));

    //categorize
    if (d == (2 * m)) D2m++;
    if (d == (2 * m + 2)) D2m2++;
    if (cd == m) Cm++;
    if (cd == (m + 1)) Cm1++;
    if ((d == (2 * m + 1)) && (cd == (m + 1))) X2m1++;
    if ((d == (2 * m + 1)) && (cd == m)) Y2m1++;
}

void SamplePairs::analyse(QImage & image, Color color, bool overlap)
{
    Q_UNUSED(overlap);

    Cm = Cm1 = D2m = D2m2 = X2m1 = Y2m1 = 0;
    mMessageLength = 0;

    for (int m = 0; m < 128; m++) {

        //pairs across the image
        for (int starty = 0; starty < image.height(); starty++) {
            for (int startx = 1; startx < image.width(); startx++) {
                this->categorize(m, startx - 1, starty, startx, starty, color, image);
            }
        }

        //pairs down the image
        for (int starty = 1; starty < image.height(); starty++) {
            for (int startx = 0; startx < image.width(); startx++) {
                this->categorize(m, startx, starty - 1, startx, starty, color, image);
            }
        }

        if ((m == 0) ? (2 * Cm <= Cm1) : (Cm <= Cm1)) return;
    }

    double a = (Cm - Cm1) / 4.0;
    double b = - ((D2m - D2m2) / 2.0 + Y2m1 - X2m1);
    double c = Y2m1 - X2m1;

    double D = b * b - 4 * a * c;

    if (D < 0) return;

    double r1 = (-b + qSqrt(D)) / (2 * a);
    double r2 = (-b - qSqrt(D)) / (2 * a);
    double r = qMin(r1, r2);

    if (r < 0) return;

    mMessageLength = r;
}
