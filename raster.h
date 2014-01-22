#ifndef RASTER_H
#define RASTER_H

#include "stegosum.h"
#include "variation.h"
#include <iostream>

class Raster : public Stegosum
{
private:

    struct statsObj {
        qint32 bitsR;
        qint32 bitsG;
        qint32 bitsB;
        qint32 total;
    };

    struct ColorPermutation {
        int code;
        int permutation[3];

        ColorPermutation() {
            code = 0;
            for (int i = 0; i < 3; i++) permutation[i] = i;
        }

        bool next() {
            code++;
            return std::next_permutation(permutation, permutation + 3);
        }

        void set(ColorPermutation & p) {
            code = p.code;
            for (int i = 0; i < 3; i++) permutation[i] = p.permutation[i];
        }

        void setCode(int code) {
            reset();
            while (this->code != code) next();
        }

        void reset() {
            code = 0;
            for (int i = 0; i < 3; i++) permutation[i] = i;
        }

        void print() {
            std::cout << "color permutation " << code << " :";
            for (int i = 0; i < 3; i++) std::cout << " " << permutation[i];
            std::cout << std::endl;
        }
    };

public:
    Raster(const QString & name);
    bool Encode();
    bool Decode();
    void save(QString &name);
    QPair<QImage, QImage> scale(float factor);
    void setSelected(Utils::Color color);
    QPair<QImage, QImage> get(Utils::Color color) {
        return qMakePair(mSelectionIn[color], mSelectionOut[color]);
    }

private:
    statsObj mStats[4];
    QMap<Utils::Color, QImage> mSelectionIn;
    QMap<Utils::Color, QImage> mSelectionOut;

    void convertToLSB(QImage & image, Utils::Color color);
    void numToBits(quint32 msgSize, quint32 shift, QVector<bool> & msgBoolVect);
    void setSeed(QImage & image, quint16 key);
    void fillPixelVector(QVector<QRgb *> & pixVect, QImage & image);
    quint32 bitsToNum(qint32 numBitsCount, QVector<bool> & msgBoolVect);
    QRgb * nextPixel(qint32 &start, QVector<QRgb *> &pixVect);
    void setStats(statsObj * stats, quint8 bits, quint8 color);
    void resetStats(statsObj *stats);
    qint32 encodeLookAhead(qint32 & start, QImage & image, quint16 key, QVector<bool> & msgBVect, QVector<QRgb *> & pixVect);
    qint32 encodeLookAhead(qint32 & start, Variation & variation, ColorPermutation & permutation, QVector<bool> &msgBVect, QVector<QRgb *> & pixVect);
    qint32 decodeLookAhead(qint32 & start, qint32 numOfBitsToDecode, QVector<bool> & msgBVect, QVector<QRgb *> & pixVect);
    void moveSequence(QImage & image, quint16 key, qint32 move);
    qint32 encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, quint8 numPars, ...);
    qint32 encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, QVector<bool> & vector);
    void encodeToPixel(QRgb * pixel, quint8 toHowManyBits, Utils::colorsObj colors, QVector<bool> & vector);
    qint32 decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, quint8 numPars, ...);
    qint32 decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, qint32 bitsSum, QVector<bool> & vector);
};

#endif // RASTER_H
