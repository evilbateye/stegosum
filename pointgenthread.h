#ifndef POINTGENTHREAD_H
#define POINTGENTHREAD_H

#include <QThread>
#include <QWidget>
#include <QFileDialog>
#include <QDebug>
#include <QProgressBar>
#include <QVector>
#include <QBuffer>

#include "utils.hpp"

class PointGenThread : public QThread
{
    Q_OBJECT
public:
    enum Color {RED, GREEN, BLUE, ALL, NONE};
    enum Bits {BITS_ENCODED_0, BITS_ENCODED_1, BITS_ENCODED_2, BITS_ENCODED_3};
    struct statsObj {
        qint32 bitsR;
        qint32 bitsG;
        qint32 bitsB;
        qint32 total;
    };
    struct variationObj {
        quint8 code;
        quint8 variation[6];
        bool isSame(quint8 * array) {
            return variation[0] == array[0] && variation[1] == array[1] && variation[2] == array[2];
        }
        void set(quint8 * array, quint8 code) {
            for (quint8 i = 0; i < 3; i++) variation[i] = array[i];
            this->code = code;
        }
    };
    struct permutationObj {
        quint8 code;
        quint8 counts[3];
        quint8 permut[3];
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
        quint16 getCountsSum() { return counts[0] + counts[1] + counts[2]; }
    };

    explicit PointGenThread(QWidget *parent = 0);
    void run();
    bool Encode(QImage & image, QByteArray & secretMsg, quint16 key);
    bool Decode(QImage & image, quint16 key = 0);
    void setUp(const QImage & img, QByteArray & msg, quint16 key,
               colorsObj & color,
               bool encode = true,
               bool isCompress = false,
               bool isEncrypt = false,
               bool isLookAhead = false,
               bool isMeta = false);
    const QImage & getImg();
    inline bool isEncode() { return mEncode; }

signals:
    void succes(bool succ);
    void updateProgress(int value);
    void setMaximum(int value);
    void sendMessage(QByteArray message, bool compressed, bool encrypted);

public slots:

private:
    QImage mImg;
    QByteArray mMsg;
    quint16 mKey;
    bool mEncode;
    bool mIsCompress;
    bool mIsEncrypt;
    bool mIsLookAhead;
    bool mIsMeta;
    colorsObj mColors;
    statsObj mStats[4];
    qint32 mMetaStats;
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
    void getPermutation(quint8 code, quint8 * permutation, quint8 selector);
    void moveSequence(QImage & image, quint16 key, qint32 move);
    qint32 encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, colorsObj colors, quint8 numPars, ...);
    qint32 encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, colorsObj colors, QVector<bool> & vector);
    void encodeToPixel(QRgb * pixel, quint8 toHowManyBits, colorsObj colors, QVector<bool> & vector);
    qint32 decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, colorsObj colors, quint8 numPars, ...);
    qint32 decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, colorsObj colors, qint32 bitsSum, QVector<bool> & vector);
};

#endif // POINTGENTHREAD_H
