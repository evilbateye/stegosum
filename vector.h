#ifndef VECTOR_H
#define VECTOR_H

#define BIT_ENCODING 63
#define BE_MAX_DEC_NUM 9999999999999999999ull
#define BE_CIPHERS_COUNT 19

#include "stegosum.h"
#include <QDomDocument>
#include <cmath>

//! Modul zabezpečuje vkladanie tajnej správy do vektorových obrázkov.
/*!
Modul pre vektorovú grafiku sa špecifikuje na prácu s vektorovými obrázkami, kon-
krétne na obrázky typu svg. Modul pre vektorovú grafiku obsahuje potrebné členské
premenné pre uchovanie informácií o posunutí desatinnej čiarky pri kódovaní tajnej
správy a príznak pre výpočet maximálneho povoleného posunutia desatinnej čiarky.
 */
class Vector : public Stegosum
{
public:
    Vector(const QString & name);
    ~Vector();
    bool Encode();
    bool Decode();
    void saveStegoImg(QString &name);
    QPair<QImage, QImage> scaleImgs(float factor);
    void setSelectedImgs(Utils::DisplayImageColor color);
    QPair<QImage, QImage> getImgs(Utils::DisplayImageColor color);
    static QString digitStream(qreal number, int fppos);
    static qreal streamToReal(QString digitStream, int ffpos);

private:

    QMap<Utils::DisplayImageColor, QByteArray> mSelXmlIn;
    QMap<Utils::DisplayImageColor, QByteArray> mSelXmlOut;

    QSize mSize;

    void iluminatePoints(QByteArray & arr);
    qreal totalMessageLength(QString msg, int fpppos);
    qreal polyLineLength(QStringList & in);
    bool nextPointSecret(QDomNodeList & nodes, QString &msg, int & polylineStart, int &lineStart, qreal & c, int fppos);
    QString addToReal(qreal real, int inc, int fppos);
    void preparePolyLine(QStringList & in);
    bool linesNotParallel(QLineF & line, QLineF & nextLine);    
    QString setDigitAt(QString number, int digit, int pos = 0);
    int digitAt(QString number, int pos = 0);
    bool isLineSupported(QString path, QStringList &list, QChar &z);
    bool precisionCorrection(qreal precise, QString & A, QString & B);
    int computeDifference(qreal precise, qreal a, qreal b);
    int encodeMessage(QString &arr);
    void randomizeWord(quint64 enc, QString &arr, int ciphersC = BE_CIPHERS_COUNT, quint64 maxDec = BE_MAX_DEC_NUM);
    void decodeMessage(QByteArray & res, QString msg);
    void derandomizeWord(QVector<bool> & v, quint64 w, int take = 0, quint64 maxDec = BE_MAX_DEC_NUM, int bits = BIT_ENCODING);
    int numberOfCiphers(int bits) { return  floor(log10(((1ull << bits) - 1))) + 1; }
    quint64 power(int base, int exp) {
        quint64 r = 1;
        for (int i = 0; i < exp; i++) r *= base;
        return r;
    }
    quint64 maximumDecadicNumber(int ciphC) { return power(10, ciphC) - 1; }
    int removeZeros(QVector<bool> & v);
    int minimumBitsCount(quint64 n) {
        int bits = 1;
        for (int i = 1; i < 64; i++) {
            if ((n >> i) & 1) bits = i + 1;
        }
        return bits;
    }
};

#endif // VECTOR_H
