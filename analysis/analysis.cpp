#include "analysis.h"

Analysis::Analysis()
{
}

quint8 Analysis::getColor(Analysis::Color color, const QRgb * pixel)
{
    switch (color) {
        case Analysis::ANALYSIS_COLOR_RED: {
            return qRed(*pixel);
        }

        case Analysis::ANALYSIS_COLOR_GREEN: {
            return qGreen(*pixel);
        }

        case Analysis::ANALYSIS_COLOR_BLUE: {
            return qBlue(*pixel);
        }
    }

    return -1;
}
