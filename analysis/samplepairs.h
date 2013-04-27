#ifndef SAMPLEPAIRS_H
#define SAMPLEPAIRS_H

#include "analysis.h"

class SamplePairs : public Analysis
{
public:
    SamplePairs();
    double analyse(QImage &image, Color color);
    quint8 getColor(Color color, QRgb * pixel);
};

#endif // SAMPLEPAIRS_H
