#include <QImage>

#include "rs.h"
#include <QtCore/qmath.h>
#include <QDebug>

int RS::mMask[2][RS::mM * RS::mN] = {
    {1, 0, 0, 1},
    {0, 1, 1, 0}
};

RS::RS()
{
}

void RS::analyse(QImage & image, Color color, bool overlap) {

    mMessageLength = -1;

    //get the images sizes
    int imgx = image.width();
    int imgy = image.height();

    int startx = 0;
    int starty = 0;
    int block[mM * mN];

    double numregular, numsingular, numnegreg, numnegsing, numunusable, numnegunusable, variationB, variationP, variationN;
    numregular = numsingular = numnegreg = numnegsing = numunusable = numnegunusable = variationB = variationP = variationN = 0;

    while (startx < imgx && starty < imgy) {
        //this is done once for each mask...
        for (int m = 0; m < 2; m++) {
            //get the block of data
            int k = 0;
            for (int i = 0; i < mN; i++) {
                for (int j = 0; j < mM; j++) {
                    block[k] = *(&reinterpret_cast<QRgb *>(image.scanLine(starty + i))[startx + j]);
                    k++;
                }
            }

            //get the variation the block
            variationB = getVariation(block, mM * mN, color, mMask[m]);

            //now flip according to the mask
            flipBlock(block, mM * mN, mMask[m]);
            variationP = getVariation(block, mM * mN, color, mMask[m]);
            //flip it back
            flipBlock(block, mM * mN, mMask[m]);

            //negative mask
            invertMask(mMask[m], mM * mN);
            variationN = getVariation(block, mM * mN, color, mMask[m], true);
            invertMask(mMask[m], mM * mN);

            //now we need to work out which group each belongs to

            //positive groupings
            if(variationP > variationB)
                numregular++;
            if(variationP < variationB)
                numsingular++;
            if(variationP == variationB)
                numunusable++;

            //negative mask groupings
            if(variationN > variationB)
                numnegreg++;
            if(variationN < variationB)
                numnegsing++;
            if(variationN == variationB)
                numnegunusable++;

            //now we keep going...
        }
        //get the next position
        if(overlap)
            startx += 1;
        else
            startx += mM;

        if(startx >= (imgx - 1)){
            startx = 0;
            if(overlap)
                starty += 1;
            else
                starty += mN;
        }
        if(starty >= (imgy - 1))
            break;
    }

    //get all the details needed to derive x...
    double totalgroups = numregular + numsingular + numunusable;

    double allpixels[4];
    getAllPixelFlips(image, color, overlap, allpixels);

    double x = getX(numregular, numnegreg, allpixels[0], allpixels[2], numsingular, numnegsing, allpixels[1], allpixels[3]);

    //calculate the estimated percent of flipped pixels and message length
    double epf, ml;
    if( 2 * (x - 1) == 0)
        epf = 0;
    else
        epf = qAbs(x / (2 * (x - 1)));

    if(x - 0.5 == 0)
        ml = 0;
    else
        ml = qAbs(x / (x - 0.5));

    //now we have the number of regular and singular groups...
    //save them all...

    //these results
    double results[28];
    results[0] = numregular;
    results[1] = numsingular;
    results[2] = numnegreg;
    results[3] = numnegsing;
    results[4] = qAbs(numregular - numnegreg);
    results[5] = qAbs(numsingular - numnegsing);
    results[6] = (numregular / totalgroups) * 100;
    results[7] = (numsingular / totalgroups) * 100;
    results[8] = (numnegreg / totalgroups) * 100;
    results[9] = (numnegsing / totalgroups) * 100;
    results[10] = (results[4] / totalgroups) * 100;
    results[11] = (results[5] / totalgroups) * 100;

    //all pixel results
    results[12] = allpixels[0];
    results[13] = allpixels[1];
    results[14] = allpixels[2];
    results[15] = allpixels[3];
    results[16] = qAbs(allpixels[0] - allpixels[1]);
    results[17] = qAbs(allpixels[2] - allpixels[3]);
    results[18] = (allpixels[0] / totalgroups) * 100;
    results[19] = (allpixels[1] / totalgroups) * 100;
    results[20] = (allpixels[2] / totalgroups) * 100;
    results[21] = (allpixels[3] / totalgroups) * 100;
    results[22] = (results[16] / totalgroups) * 100;
    results[23] = (results[17] / totalgroups) * 100;

    //overall results
    results[24] = totalgroups;
    results[25] = epf;
    results[26] = ml;
    results[27] = ((imgx * imgy * 3) * ml) / 8;

    mMessageLength = results[26];
}

double RS::getX(double r, double rm, double r1, double rm1, double s, double sm, double s1, double sm1)
{
    double x = 0; //the cross point.

    double dzero = r - s; // d0 = Rm(p/2) - Sm(p/2)
    double dminuszero = rm - sm; // d-0 = R-m(p/2) - S-m(p/2)
    double done = r1 - s1; // d1 = Rm(1-p/2) - Sm(1-p/2)
    double dminusone = rm1 - sm1; // d-1 = R-m(1-p/2) - S-m(1-p/2)

    //get x as the root of the equation
    //2(d1 + d0)x^2 + (d-0 - d-1 - d1 - 3d0)x + d0 - d-0 = 0
    //x = (-b +or- sqrt(b^2-4ac))/2a
    //where ax^2 + bx + c = 0 and this is the form of the equation

    double a = 2 * (done + dzero);
    double b = dminuszero - dminusone - done - (3 * dzero);
    double c = dzero - dminuszero;

    if(a == 0)
        //take it as a straight line
        x = c / b;

    //take it as a curve
    double discriminant = qPow(b,2) - (4 * a * c);

    if(discriminant >= 0){
        double rootpos = ((-1 * b) + qSqrt(discriminant)) / (2 * a);
        double rootneg = ((-1 * b) - qSqrt(discriminant)) / (2 * a);

        //return the root with the smallest absolute value (as per paper)
        if(qAbs(rootpos) <= qAbs(rootneg))
            x = rootpos;
        else
            x = rootneg;
    }else{
        //maybe it's not the curve we think (straight line)
        double cr = (rm - r) / (r1 - r + rm - rm1);
        double cs = (sm - s) / (s1 -s + sm - sm1);
        x = (cr + cs) / 2;
    }

    if(x == 0){
        double ar = ((rm1 - r1 + r - rm) + (rm - r) / x) / (x - 1);
        double as = ((sm1 - s1 + s - sm) + (sm - s) / x) / (x - 1);
        if((as > 0) | (ar < 0)){
            //let's assume straight lines again...
            double cr = (rm - r) / (r1 - r + rm - rm1);
            double cs = (sm - s) / (s1 -s + sm - sm1);
            x = (cr + cs) / 2;
        }
    }
    return x;
}

void RS::getAllPixelFlips(QImage image, Analysis::Color color, bool overlap, double * results)
{
    //setup the mask for everything...
    int allmask[mM * mN];
    for (int i = 0; i < (mM * mN); i++) {
        allmask[i] = 1;
    }

    //now do the same as the doAnalysis() method

    //get the images sizes
    int imgx = image.width();
    int imgy = image.height();

    int startx = 0;
    int starty = 0;
    int block[mM * mN];

    double numregular, numsingular, numnegreg, numnegsing, numunusable, numnegunusable, variationB, variationP, variationN;
    numregular = numsingular = numnegreg = numnegsing = numunusable = numnegunusable = variationB = variationP = variationN = 0;

    while (startx < imgx && starty < imgy) {
        //this is done once for each mask...
        for (int m = 0; m < 2; m++) {
            //get the block of data
            int k = 0;
            for (int i = 0; i < mN; i++) {
                for (int j = 0; j < mM; j++) {
                    block[k] = *(&reinterpret_cast<QRgb *>(image.scanLine(starty + i))[startx + j]);
                    k++;
                }
            }

            //flip all the pixels in the block (NOTE: THIS IS WHAT'S DIFFERENT
            //TO THE OTHER doAnalysis() METHOD)
            flipBlock(block, mM * mN, allmask);

            //get the variation the block
            variationB = getVariation(block, mM * mN, color, mMask[m]);

            //now flip according to the mask
            flipBlock(block, mM * mN, mMask[m]);
            variationP = getVariation(block, mM * mN, color, mMask[m]);
            //flip it back
            flipBlock(block, mM * mN, mMask[m]);

            //negative mask
            invertMask(mMask[m], mM * mN);
            variationN = getVariation(block, mM * mN, color, mMask[m], true);
            invertMask(mMask[m], mM * mN);

            //now we need to work out which group each belongs to

            //positive groupings
            if(variationP > variationB)
                numregular++;
            if(variationP < variationB)
                numsingular++;
            if(variationP == variationB)
                numunusable++;

            //negative mask groupings
            if(variationN > variationB)
                numnegreg++;
            if(variationN < variationB)
                numnegsing++;
            if(variationN == variationB)
                numnegunusable++;

            //now we keep going...
        }
        //get the next position
        if(overlap)
            startx += 1;
        else
            startx += mM;

        if(startx >= (imgx - 1)){
            startx = 0;
            if(overlap)
                starty += 1;
            else
                starty += mN;
        }
        if(starty >= (imgy - 1))
            break;
    }

    //save all the results (same order as before)
    results[0] = numregular;
    results[1] = numsingular;
    results[2] = numnegreg;
    results[3] = numnegsing;
}

void RS::invertMask(int * mask, int maskLen)
{
    for (int i = 0; i < maskLen; i++) {
        mask[i] = mask[i] * -1;
    }
}

int RS::invertLSB(int colorByte)
{
    //FIXME 0 -> 255 a naopak?
    if (colorByte == 255) return 256;
    if (colorByte == 256) return 255;
    return (negateLSB(colorByte + 1) - 1);
}

quint8 RS::negateLSB(quint8 colorByte)
{
    if ((colorByte & 0x01) == 0x01) {
        return (colorByte & 0xFE);
    } else {
        return (colorByte | 0x01);
    }
}

void RS::flipBlock(int * block, int blockLen, int * mask)
{
    //if the mask is true, negate every LSB
    for (int i = 0; i < blockLen; i++) {
        //get the colour
        int red = getColorRS(Analysis::ANALYSIS_COLOR_RED, block[i]);
        int green = getColorRS(Analysis::ANALYSIS_COLOR_GREEN, block[i]);
        int blue = getColorRS(Analysis::ANALYSIS_COLOR_BLUE, block[i]);

        if (mask[i] == 1) {
            //negate their LSBs
            red = negateLSB(red);
            green = negateLSB(green);
            blue = negateLSB(blue);
//            int tmp2 = (0xff << 24) | ((red  & 0xff) << 16) | ((green & 0xff) << 8) | ((blue & 0xff));
            block[i] = qRgba(red, green, blue, 0xFF);
        } else if (mask[i] == -1) {
            //negate their LSBs
            red = invertLSB(red);
            green = invertLSB(green);
            blue = invertLSB(blue);
            block[i] = qRgba(red, green, blue, 0xFF);
        }

        //build a new pixel
        //FIXME alpha matters?
//        block[i] = qRgba(red, green, blue, qAlpha(block[i]));
    }
}

int RS::getColorRS(Analysis::Color color, int pixel)
{
    switch (color) {
        case Analysis::ANALYSIS_COLOR_RED: {
            return ((pixel >> 16) & 0xff);
        }

        case Analysis::ANALYSIS_COLOR_GREEN: {
            return ((pixel >> 8) & 0xff);
        }

        case Analysis::ANALYSIS_COLOR_BLUE: {
            return (pixel & 0xff);
        }
    }

    return -1;
}

double RS::getVariation(int * block, int blockLen, Analysis::Color color, int * mask, bool negativeVariation)
{
    double var = 0;
    int colour1, colour2;
    for (int i = 0; i < blockLen; i = i + 4) {
        colour1 = getColorRS(color, block[0 + i]);
        colour2 = getColorRS(color, block[1 + i]);
        if (negativeVariation) {
            if(mask[0 + i] == -1)
                colour1 = invertLSB(colour1);
            if(mask[1 + i] == -1)
                colour2 = invertLSB(colour2);
        }
        var += qAbs(colour1 - colour2);

        colour1 = getColorRS(color, block[3 + i]);
        colour2 = getColorRS(color, block[2 + i]);
        if (negativeVariation) {
            if(mask[3 + i] == -1)
                colour1 = invertLSB(colour1);
            if(mask[2 + i] == -1)
                colour2 = invertLSB(colour2);
        }
        var += qAbs(colour1 - colour2);

        colour1 = getColorRS(color, block[1 + i]);
        colour2 = getColorRS(color, block[3 + i]);
        if (negativeVariation) {
            if(mask[1 + i] == -1)
                colour1 = invertLSB(colour1);
            if(mask[3 + i] == -1)
                colour2 = invertLSB(colour2);
        }
        var += qAbs(colour1 - colour2);

        colour1 = getColorRS(color, block[2 + i]);
        colour2 = getColorRS(color, block[0 + i]);
        if (negativeVariation) {
            if(mask[2 + i] == -1)
                colour1 = invertLSB(colour1);
            if(mask[0 + i] == -1)
                colour2 = invertLSB(colour2);
        }
        var += qAbs(colour1 - colour2);
    }
    return var;
}
