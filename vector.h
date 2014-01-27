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
    void save(QString &name);
    QPair<QImage, QImage> scale(float factor);
    void setSelected(Utils::Color color);
    QPair<QImage, QImage> get(Utils::Color color);

private:

    QMap<Utils::Color, QByteArray> mSelXmlIn;
    QMap<Utils::Color, QByteArray> mSelXmlOut;

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
