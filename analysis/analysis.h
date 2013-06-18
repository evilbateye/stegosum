#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <QImage>

class Analysis
{
public:
    enum Color {
        ANALYSIS_COLOR_RED,
        ANALYSIS_COLOR_GREEN,
        ANALYSIS_COLOR_BLUE
    };

    Analysis();
    virtual void analyse(QImage & image, Color color, bool overlap) = 0;
    quint8 getColor(Color color, QRgb * pixel);
    double getMessageLength() { return mMessageLength; }

protected:
    double mMessageLength;
};

#endif // ANALYSIS_H
