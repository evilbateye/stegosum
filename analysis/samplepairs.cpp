#include <QImage>
#include <QColor>
#include <QtCore/qmath.h>

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
double SamplePairs::analyse(QImage &image, Color color)
{
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

    return x;
}

quint8 SamplePairs::getColor(Analysis::Color color, QRgb * pixel)
{
    switch (color) {
        case Analysis::ANALYSIS_COLOR_RED: {
            return qRed(*pixel);
        }

        case Analysis::ANALYSIS_COLOR_GREEN: {
            return qGreen(*pixel);
        }

        case Analysis::ANALYSIS_COLOR_BLUE: {
            return qBlue(*pixel);
        }
    }

    return -1;
}
