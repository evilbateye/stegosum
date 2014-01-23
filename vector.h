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

    struct Number {
        bool mIsPositive;
        QStringList mFloatingPoint;

        Number(const QString & string) {
            mFloatingPoint = string.split(".");
            if (mFloatingPoint.size() < 2) mFloatingPoint << "";
            normalize();
        }
        Number(const QStringList & list) : mFloatingPoint(list) {}
        QString toString() { return mFloatingPoint.join("."); }
        int size() { return mFloatingPoint.join("").size(); }
        const QChar at(int i) { return mFloatingPoint.join("").at(i); }
        void normalize();
        void fillzero(Number & other);
        Number operator- (const Number & other);
        void clean();
        void swap(Number & other);
        bool operator> (const Number & other);
        bool operator< (const Number & other);
        bool operator>= (const Number & other);
        bool operator<= (const Number & other);
        bool operator== (const Number & other);
        Number & operator= (const Number & other);
    };

    void iluminatePoints(QByteArray & arr);
    qreal totalMessageLength(QString msg, int fpppos);
    qreal polyLineLength(QStringList & in);
    bool nextPointSecret(QDomNodeList & nodes, QString &msg, int & polylineStart, int &lineStart, qreal & c, int fppos);
    QString addToReal(qreal real, int inc, int fppos);
    void preparePolyLine(QStringList & in);
    bool linesNotParallel(QLineF & line, QLineF & nextLine);    
    QString setEven(QString number, bool isEven, int &direction);
    QString setEven(QString number, bool isEven);
    QString setDigitAt(QString number, int digit, int pos = 0);
    bool isEven(double number);
    bool isEven(QString number);
    int digitAt(QString number, int pos = 0);
    void setMainLine(QLineF & line, QStringList & in, int & index);
    QString secretFromLine(int lineNumber, QLineF & mainLine, QStringList & in);
    bool isLineSupported(QStringList & in);
};

#endif // VECTOR_H
