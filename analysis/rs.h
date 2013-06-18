#ifndef RS_H
#define RS_H

#include "analysis.h"

class RS : public Analysis
{
public:
    RS();
    void analyse(QImage &image, Color color, bool overlap);
    double getVariation(int *block, int blockLen, Color color, int *mask, bool negativeVariation = false);
    void flipBlock(int *block, int blockLen, int *mask);
    quint8 negateLSB(quint8 colorByte);
    int invertLSB(int colorByte);
    void invertMask(int * mask, int maskLen);
    void getAllPixelFlips(QImage image, Analysis::Color colour, bool overlap, double * results);
    double getX(double r, double rm, double r1, double rm1, double s, double sm, double s1, double sm1);
    int getColorRS(Analysis::Color color, int pixel);
private:
    static int const mM = 2;
    static int const mN = 2;
    static int mMask[2][mM * mN];
};

#endif // RS_H
