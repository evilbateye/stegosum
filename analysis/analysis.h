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
    virtual double analyse(QImage & image, Color color) = 0;
};

#endif // ANALYSIS_H
