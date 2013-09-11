#ifndef SAMPLEPAIRS_H
#define SAMPLEPAIRS_H

#include "analysis.h"

class SamplePairs : public Analysis
{
public:
    SamplePairs();
    void analyse(const QImage &image, Color color, bool overlap = false);
    void analyseOld(const QImage &image, Color color, bool overlap = false);

private:
    int Cm, Cm1, D2m, D2m2, X2m1, Y2m1;
    void categorize(int m, int x1, int y1, int x2, int y2, Color color, const QImage &image);
};

#endif // SAMPLEPAIRS_H
