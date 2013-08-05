#include "utils.hpp"

Utils::Utils()
{
}

qint32 Utils::pixelsNeeded(qint32 bits, quint8 selectedColors, quint8 toHowManyBits) {
    qint32 pixels = bits / (selectedColors * toHowManyBits);
    if (bits % (selectedColors * toHowManyBits)) pixels++;
    return pixels;
}
