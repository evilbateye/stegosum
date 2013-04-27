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
    explicit PointGenThread(QWidget *parent = 0);
    void run();
    bool Encode(QImage & image, QByteArray & secretMsg, quint16 key);
    bool Decode(QImage & image, quint16 key = 0);
    void setUp(const QImage & img, QByteArray & msg, quint16 key, bool encode = true,
               bool isCompress = false,
               bool isEncrypt = false,
               bool isEncodeMax = false,
               Color color = NONE);
    const QImage & getImg();

    inline bool isEncode()
    {
        return mEncode;
    }

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
    bool mIsEncodeMax;
    Color mColor;

    void shuffle(qint32 msgSizeB, QImage & image, quint16 key, QVector<QPoint> & pointsList);
    void shuffle2(qint32 msgSizeB, QImage & image, QList<QRgb *> & pointsList, QVector<QRgb *> & pixVect, quint8 offset = 0);
    void numToBits(quint32 msgSize, quint32 shift, QVector<bool> & msgBoolVect);
    quint32 bitsToNum(quint32 numBitsCount, QVector<bool> & msgBoolVect, quint32 shift = 0);


};

#endif // POINTGENTHREAD_H
