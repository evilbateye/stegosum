#ifndef VECTOR_H
#define VECTOR_H

#include "stegosum.h"
#include <QDomDocument>

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
    bool isLineSupported(QStringList & in);
    bool precisionCorrection(qreal precise, QString & A, QString & B);
    int computeDifference(qreal precise, qreal a, qreal b);
};

#endif // VECTOR_H
