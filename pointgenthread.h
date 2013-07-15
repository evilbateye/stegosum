#ifndef POINTGENTHREAD_H
#define POINTGENTHREAD_H

#include <QThread>
#include <QWidget>
#include <QFileDialog>
#include <QDebug>
#include <QProgressBar>
#include <QVector>
#include <QBuffer>

class PointGenThread : public QThread
{
    Q_OBJECT
public:
    enum Color {RED, GREEN, BLUE, ALL, NONE};
    enum Bits {BITS_ENCODED_0, BITS_ENCODED_1, BITS_ENCODED_2, BITS_ENCODED_3};
    struct lookAheadObj {
        quint32 bitsR;
        quint32 bitsG;
        quint32 bitsB;
        quint32 total;
    };
    explicit PointGenThread(QWidget *parent = 0);
    void run();
    bool Encode(QImage & image, QByteArray & secretMsg, quint16 key);
    bool Decode(QImage & image, quint16 key = 0);
    void setUp(const QImage & img, QByteArray & msg, quint16 key,
               bool encode = true,
               bool isCompress = false,
               bool isEncrypt = false,
               bool isLookAhead = false,
               bool colorR = false,
               bool colorG = false,
               bool colorB = false);
    const QImage & getImg();
    inline bool isEncode() { return mEncode; }
    static quint32 getMessageSizeInPixels(quint32 messageSizeInBits, quint8 numOfSelColors);

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
    bool mColors[3];
    quint8 mNumOfSelectedColors;
    lookAheadObj mLAobj[4];
    void shuffle(qint32 msgSizeB, QImage & image, quint16 key, QVector<QPoint> & pointsList);
    void generatePixelPoints(qint32 msgSizeB, QList<QRgb *> & pointsList, QVector<QRgb *> & pixVect, quint8 offset = 0);
    bool lookAheadEncodeAlgorithm(QVector<bool> &msgBVect, QVector<QRgb *> & pixVect, quint8 offset = 0);
    quint8 getEmbeddedLookAheadColor(QVector<bool> & msgBoolVect, quint8 colorVal, Color colorType);
    void lookAheadStatistics(Bits bits, Color color);
    void resetLookAheadStatistics();
    void encodeSettings(QList<QRgb *> & pointsList);
    void numToBits(quint32 msgSize, quint32 shift, QVector<bool> & msgBoolVect);
    void setSeed(QImage & image, quint16 key);
    void fillPixelVector(QVector<QRgb *> & pixVect, QImage & image);
    quint8 getNumOfSelectedColors(bool * colors);
    void decodeSettings(QList<QRgb *> &points, bool * settings);
    quint32 bitsToNum(quint32 numBitsCount, QVector<bool> & msgBoolVect, quint32 shift = 0);
};

#endif // POINTGENTHREAD_H
