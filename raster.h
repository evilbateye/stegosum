#ifndef RASTER_H
#define RASTER_H

#include "stegosum.h"

class Raster : public Stegosum
{
private:
    struct statsObj {
        qint32 bitsR;
        qint32 bitsG;
        qint32 bitsB;
        qint32 total;
    };
    struct variationObj {
        int code;
        int variation[6];
        //FIXME delete
        bool isSame(int * array) {
            return variation[0] == array[0] && variation[1] == array[1] && variation[2] == array[2];
        }
        void set(int * array, int code) {
            for (quint8 i = 0; i < 3; i++) variation[i] = array[i];
            this->code = code;
        }
    };
    struct permutationObj {
        int code;
        int counts[3];
        int permut[3];
        void set(permutationObj & p) {
            code = p.code;
            for (qint8 i = 0; i < 3; i++) {
                counts[i] = p.counts[i];
                permut[i] = p.permut[i];
            }
        }
        void clearCounts() {
            counts[0] = 0;
            counts[1] = 0;
            counts[2] = 0;
        }
        int getCountsSum() { return counts[0] + counts[1] + counts[2]; }
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
    qint32 mMetaStats;

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
    qint32 encodeLookAhead(qint32 & start, variationObj variation, bool isMeta, QVector<bool> &msgBVect, QVector<QRgb *> & pixVect);
    qint32 decodeLookAhead(qint32 & start, qint32 numOfBitsToDecode, QVector<bool> & msgBVect, QVector<QRgb *> & pixVect);
    void getPermutation(int code, int *permutation, int selector);
    void moveSequence(QImage & image, quint16 key, qint32 move);
    qint32 encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, quint8 numPars, ...);
    qint32 encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, QVector<bool> & vector);
    void encodeToPixel(QRgb * pixel, quint8 toHowManyBits, Utils::colorsObj colors, QVector<bool> & vector);
    qint32 decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, quint8 numPars, ...);
    qint32 decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, qint32 bitsSum, QVector<bool> & vector);
};

#endif // RASTER_H
