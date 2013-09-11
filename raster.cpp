#include "raster.h"

Raster::Raster(const QString &name)
{
    mSelectionIn[Utils::COLOR_NONE].load(name);
    mSelectionIn[Utils::COLOR_NONE] = mSelectionIn[Utils::COLOR_NONE].convertToFormat(QImage::Format_ARGB32);
    mSelColor = Utils::COLOR_NONE;
    mIsRaster = true;
}

QPair<QImage, QImage> Raster::scale(float factor) {
    return qMakePair(mSelectionIn[mSelColor].scaled(mSelectionIn[mSelColor].size() * factor, Qt::IgnoreAspectRatio, Qt::FastTransformation),
                 mSelectionOut[mSelColor].scaled(mSelectionOut[mSelColor].size() * factor, Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

void Raster::setSelected(Utils::Color color) {

    if (color == Utils::COLOR_PREV) color = mSelColor;

    mSelColor = color;

    if (color == Utils::COLOR_NONE) return;

    if (mSelectionIn[color].isNull()) {
        mSelectionIn[color] = mSelectionIn[Utils::COLOR_NONE];
        convertToLSB(mSelectionIn[color], color);
    }

    if (mSelectionOut[color].isNull() && !mSelectionOut[Utils::COLOR_NONE].isNull()) {
        mSelectionOut[color] = mSelectionOut[Utils::COLOR_NONE];
        convertToLSB(mSelectionOut[color], color);
    }
}

void Raster::convertToLSB(QImage & image, Utils::Color color) {
    for (int j = 0; j < image.height(); j++) {
        for (int i = 0; i < image.width(); i++) {

            QRgb * pixel = (&reinterpret_cast<QRgb *>(image.scanLine(j))[i]);

            quint8 red = 0x00;
            quint8 green = 0x00;
            quint8 blue = 0x00;

            switch (color) {
                case Utils::COLOR_RED: {
                    red = qRed(* pixel) & 0x01;
                    if (red == 0x01) red = 0xFF;
                    break;
                }

                case Utils::COLOR_GREEN: {
                    green = qGreen(* pixel) & 0x01;
                    if (green == 0x01) green = 0xFF;
                    break;
                }

                case Utils::COLOR_BLUE: {
                    blue = qBlue(* pixel) & 0x01;
                    if (blue == 0x01) blue = 0xFF;
                    break;
                }

                case Utils::COLOR_ALL: {
                    red = qRed(* pixel) & 0x01;
                    if (red == 0x01) red = 0xFF;

                    green = qGreen(* pixel) & 0x01;
                    if (green == 0x01) green = 0xFF;

                    blue = qBlue(* pixel) & 0x01;
                    if (blue == 0x01) blue = 0xFF;
                    break;
                }

                case Utils::COLOR_NONE: break;
                default: break;
            }

            (* pixel) = qRgba(red, green, blue, 0xFF);
        }
    }
}

void Raster::numToBits(quint32 msgSize, quint32 shift, QVector<bool> & msgBoolVect)
{
    quint32 sizeMask;
    for (quint32 j = 0; j < shift; j++) {
        sizeMask = (0x00000001 << j);
        msgBoolVect.push_back((msgSize & sizeMask) == sizeMask);
    }
}

quint32 Raster::bitsToNum(qint32 numBitsCount, QVector<bool> & msgBoolVect)
{
    if (numBitsCount > msgBoolVect.size()) return 0;

    quint32 msgSize = 0x00000000;

    for (qint32 i = 0; i < numBitsCount; i++) {
        msgSize |= (msgBoolVect.first() << i);
        msgBoolVect.pop_front();
    }

    return msgSize;
}

QRgb * Raster::nextPixel(qint32 & start, QVector<QRgb *> & pixVect) {
    if (start < SUFFLEEOFFSET) return 0;

    quint32 pos = qrand() % (start + 1 - SUFFLEEOFFSET) + SUFFLEEOFFSET;
    QRgb * pixel = pixVect[pos];
    qSwap(pixVect[start--], pixVect[pos]);
    return pixel;
}

qint32 Raster::encodeLookAhead(qint32 & start, variationObj variation, bool isMeta, QVector<bool> & msgBVect, QVector<QRgb *> & pixVect)
{
    qint32 msgPtr = 0;
    qint32 oldStart = start;
    resetStats(mStats);
    mMetaStats = 0;

    encodeToPixel(start, pixVect, 2, Utils::colorsObj(true, true, true), 4, 1, mIsMeta, NUM_OF_VARIATIONS_BITS, variation.code);

    if (!isMeta) {
        while (msgPtr < msgBVect.size()) {
            QRgb * pixel = nextPixel(start, pixVect);
            if (!pixel) return 0;

            quint8 updatedColors[3];
            updatedColors[0] = qRed(*pixel);
            updatedColors[1] = qGreen(*pixel);
            updatedColors[2] = qBlue(*pixel);

            qint8 perPixel = 0;
            for (quint8 j = 0; j < 3; j++) {
                quint8 lookAhead = updatedColors[j] >> 2;

                qint8 perColor;
                for (perColor = 0; perColor < 3; perColor++) {
                    qint32 index = msgPtr + perPixel + perColor;
                    if (index >= msgBVect.size()) break;
                    if (msgBVect[index] != ((lookAhead >> variation.variation[perColor]) & 0x01)) break;
                }
                setStats(mStats, perColor, j);

                perPixel += perColor;

                updatedColors[j] = (updatedColors[j] & 0xFC) | perColor;
            }

            msgPtr += perPixel;

            *pixel = qRgba(updatedColors[0], updatedColors[1], updatedColors[2], qAlpha(*pixel));
        }

        return oldStart - start;
    }

    while (msgPtr < msgBVect.size()) {

        QRgb * meta = nextPixel(start, pixVect);
        if (!meta) return 0;
        mMetaStats++;

        quint8 permutationCodes[2] = {0, 0};

        for (quint8 i = 0; i < 2; i++) {

            QRgb * pixel = nextPixel(start, pixVect);
            if (!pixel) return 0;

            quint8 updatedColors[3];
            updatedColors[0] = qRed(*pixel);
            updatedColors[1] = qGreen(*pixel);
            updatedColors[2] = qBlue(*pixel);

            permutationObj selected;
            permutationObj tmp = {0, {0, 0, 0}, {0, 1, 2}};
            qint8 max = -1;

            do {
                tmp.clearCounts();

                qint8 perPixel = 0;
                quint8 lookAhead;
                for (quint8 j = 0; j < 3; j++) {
                    lookAhead = updatedColors[tmp.permut[j]] >> 2;

                    qint32 index;
                    for (qint8 k = 0; k < 3; k++) {
                        index = msgPtr + perPixel + k;
                        if (index >= msgBVect.size()) break;
                        if (msgBVect[index] != ((lookAhead >> variation.variation[k]) & 0x01)) break;
                        tmp.counts[j]++;
                    }

                    perPixel += tmp.counts[j];
                }

                if (perPixel > max) {
                    max = perPixel;
                    selected.set(tmp);
                }
                tmp.code++;

            } while (std::next_permutation(tmp.permut, tmp.permut + 3));

            permutationCodes[i] = selected.code;
            msgPtr += selected.getCountsSum();

            for (quint8 j = 0; j < 3; j++) {
                updatedColors[selected.permut[j]] = (updatedColors[selected.permut[j]] & 0xFC) | selected.counts[j];
                setStats(mStats, selected.counts[j], j);
            }

            *pixel = qRgba(updatedColors[0], updatedColors[1], updatedColors[2], qAlpha(*pixel));
        }

        QVector<bool> metaVector;
        numToBits(permutationCodes[0], 3, metaVector);
        numToBits(permutationCodes[1], 3, metaVector);
        encodeToPixel(meta, 2, Utils::colorsObj(true, true, true), metaVector);
    }

    return oldStart - start;
}

void Raster::moveSequence(QImage & image, quint16 key, qint32 move) {
    setSeed(image, key);
    for (qint32 i = 0; i < move; i++) qrand();
}

qint32 Raster::encodeLookAhead(qint32 & start, QImage & image, quint16 key, QVector<bool> & msgBVect, QVector<QRgb *> & pixVect)
{
    quint8 varis[6] = {0, 1, 2, 3, 4, 5};
    variationObj tmpVar = {0, {0, 0, 0}};
    variationObj selectedVar = {0, {0, 0, 0}};
    qint32 offset = pixVect.size() - 1 - start;

    int c = 0;
    qint32 minimum = pixVect.size();
    qint32 numOfGenPoints = 0;
    do {
        if (!tmpVar.isSame(varis)) {
            tmpVar.set(varis, c++);

            moveSequence(image, key, offset);

            QVector<QRgb *> tmpPixVect(pixVect);
            qint32 tmpStart = start;

            numOfGenPoints = encodeLookAhead(tmpStart, tmpVar, mIsMeta, msgBVect, tmpPixVect);

            if (numOfGenPoints && numOfGenPoints < minimum) {
                minimum = numOfGenPoints;
                selectedVar.set(tmpVar.variation, tmpVar.code);
            }
        }
    } while (std::next_permutation(varis, varis + 6));

    if (minimum == pixVect.size()) return 0;

    moveSequence(image, key, offset);

    return encodeLookAhead(start, selectedVar, mIsMeta, msgBVect, pixVect);
}

void Raster::getPermutation(quint8 code, quint8 * permutation, quint8 selector)
{
    quint8 i = 0;
    variationObj tmpVar = {0, {0, 0, 0}};

    do {
        if (tmpVar.isSame(permutation)) continue;
        tmpVar.set(permutation, i);
        if (code == i++) break;
    } while (std::next_permutation(permutation, permutation + selector));
}

qint32 Raster::decodeLookAhead(qint32 & start, qint32 numOfBitsToDecode, QVector<bool> & msgBVect, QVector<QRgb *> & pixVect)
{
    qint32 oldStart = start;

    bool isMeta = false;
    quint8 variationCode = 0;
    decodeFromPixel(start, pixVect, 2, Utils::colorsObj(true, true, true), 4, 1, &isMeta, 7, &variationCode);

    variationObj v = {variationCode, {0, 1, 2, 3, 4, 5}};
    getPermutation(v.code, v.variation, 6);

    if (!isMeta) {

        while (msgBVect.size() < numOfBitsToDecode) {
            QRgb * pixel = nextPixel(start, pixVect);
            if (!pixel) return 0;

            quint8 updatedColors[3];
            updatedColors[0] = qRed(*pixel);
            updatedColors[1] = qGreen(*pixel);
            updatedColors[2] = qBlue(*pixel);

            for (quint8 j = 0; j < 3; j++) {
                for (quint8 k = 0; k < (updatedColors[j] & 0x03); k++) {
                    msgBVect.push_back((updatedColors[j] >> (2 + v.variation[k])) & 0x01);
                }
            }
        }

        return oldStart - start;
    }

    while (msgBVect.size() < numOfBitsToDecode) {
        quint8 permutationCodes[2] = {0, 0};
        decodeFromPixel(start, pixVect, 2, Utils::colorsObj(true, true, true), 4, 3, &permutationCodes[0], 3, &permutationCodes[1]);

        for (quint8 i = 0; i < 2; i++) {

            QRgb * pixel = nextPixel(start, pixVect);
            if (!pixel) return 0;

            quint8 updatedColors[3];
            updatedColors[0] = qRed(*pixel);
            updatedColors[1] = qGreen(*pixel);
            updatedColors[2] = qBlue(*pixel);

            permutationObj p = {permutationCodes[i], {0, 0, 0}, {0, 1, 2}};
            getPermutation(p.code, p.permut, 3);

            for (quint8 j = 0; j < 3; j++) {
                for (quint8 k = 0; k < (updatedColors[p.permut[j]] & 0x03); k++) {
                    msgBVect.push_back((updatedColors[p.permut[j]] >> (2 + v.variation[k])) & 0x01);
                }
            }
        }
    }

    return oldStart - start;
}

void Raster::resetStats(statsObj * stats)
{
    for (int i = 0; i < 4; i++) {
        stats[i].bitsR = 0;
        stats[i].bitsG = 0;
        stats[i].bitsB = 0;
        stats[i].total = 0;
    }
}

void Raster::setStats(statsObj * stats, quint8 bits, quint8 color)
{
    switch (color) {
        case 0: { stats[bits].bitsR++; break;}
        case 1: { stats[bits].bitsG++; break;}
        case 2: { stats[bits].bitsB++; break;}
    }
    stats[bits].total++;
}

void Raster::setSeed(QImage & image, quint16 key)
{
    QRgb * first2pixels = (QRgb *) image.bits();
    quint32 seed = key ^ (first2pixels[0] + first2pixels[1]);
    qsrand(seed);
}

void Raster::fillPixelVector(QVector<QRgb *> & pixVect, QImage & image)
{
    for (int i = 0; i < image.height(); i++) {
        for (int j = 0; j < image.width(); j++) {
            pixVect[i * image.width() + j] = (&reinterpret_cast<QRgb *>(image.scanLine(i))[j]);
        }
    }
}

void Raster::encodeToPixel(QRgb * pixel, quint8 toHowManyBits, Utils::colorsObj colors, QVector<bool> & vector) {
    quint8 updatedColors[3];
    updatedColors[0] = qRed(*pixel);
    updatedColors[1] = qGreen(*pixel);
    updatedColors[2] = qBlue(*pixel);

    for (int j = 0; j < 3; j++) {

        if (!colors.rgb[j]) continue;

        updatedColors[j] &= (0xFF << toHowManyBits);

        for (int k = 0; k < toHowManyBits; k++) {
            if (vector.isEmpty()) break;
            updatedColors[j] |= (vector.front() << k);
            vector.pop_front();
        }
    }

    *pixel = qRgba(updatedColors[0], updatedColors[1], updatedColors[2], qAlpha(*pixel));
}

qint32 Raster::encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, QVector<bool> & vector) {

    qint32 numPixels = Utils::pixelsNeeded(vector.size(), colors.numOfselected, toHowManyBits);

    for (qint32 i = 0; i < numPixels; i++) {
        QRgb * pixel = nextPixel(start, pixVect);
        if (!pixel) return 0;
        encodeToPixel(pixel, toHowManyBits, colors, vector);
    }

    return numPixels;
}

qint32 Raster::encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, quint8 numPars, ...) {
    if (numPars % 2) return 0;

    numPars = numPars / 2;

    va_list arguments;
    va_start(arguments, numPars);

    QVector<bool> vector;
    for (int i = 0; i < numPars; i++) {
        quint8 bits = va_arg(arguments, int);
        quint32 val = va_arg(arguments, int);
        numToBits(val, bits, vector);
    }
    va_end(arguments);

    return encodeToPixel(start, pixVect, toHowManyBits, colors, vector);
}

qint32 Raster::decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, quint8 numPars, ...) {
    if (numPars % 2) return 0;

    numPars = numPars / 2;

    quint8 bits[numPars];
    qint32 * vals[numPars];

    quint32 bitsSum = 0;

    va_list arguments;
    va_start(arguments, numPars);
    for (int i = 0; i < numPars; i++) {
        bits[i] = va_arg(arguments, int);
        vals[i] = va_arg(arguments, int*);
        bitsSum += bits[i];
    }
    va_end(arguments);

    QVector<bool> vector;
    qint32 numPixels = decodeFromPixel(start, pixVect, toHowManyBits, colors, bitsSum, vector);

    for (int i = 0; i < numPars; i++) {
        *vals[i] = bitsToNum(bits[i], vector);
    }

    return numPixels;
}

qint32 Raster::decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::colorsObj colors, qint32 bitsSum, QVector<bool> & vector) {

    qint32 numPixels = Utils::pixelsNeeded(bitsSum, colors.numOfselected, toHowManyBits);

    for (qint32 i = 0; i < numPixels; i++) {
        QRgb * pixel = nextPixel(start, pixVect);
        if (!pixel) return 0;

        quint8 updatedColors[3];
        updatedColors[0] = qRed(*pixel);
        updatedColors[1] = qGreen(*pixel);
        updatedColors[2] = qBlue(*pixel);

        for (int j = 0; j < 3; j++) {

            if (!colors.rgb[j]) continue;

            for (int k = 0; k < toHowManyBits; k++) {
                if (vector.size() >= bitsSum) break;
                vector.push_back((updatedColors[j] >> k) & 0x01);
            }
        }
    }

    return numPixels;
}

void Raster::save(QString &name) {
    mSelectionOut[Utils::COLOR_NONE].save(name);
}

bool Raster::Encode()
{
    /*QImage simg("/home/evilbateye/Pictures/1318099542371.png");
    QByteArray msgBytes;
    QBuffer buffer(&msgBytes);
    buffer.open(QIODevice::WriteOnly);
    qDebug()<<simg.save(&buffer, "PNG");*/

    mSelectionOut[Utils::COLOR_NONE] = mSelectionIn[Utils::COLOR_NONE];
    mSelectionOut[Utils::COLOR_RED] = QImage();
    mSelectionOut[Utils::COLOR_GREEN] = QImage();
    mSelectionOut[Utils::COLOR_BLUE] = QImage();
    mSelectionOut[Utils::COLOR_ALL] = QImage();

    if (!mColors.numOfselected) {
        emit writeToConsole("[Raster] No colors selected. Nowhere to encode.\n");
        return false;
    }

    setSeed(mSelectionOut[Utils::COLOR_NONE], mKey);

    QVector<QRgb *> pixVect(mSelectionOut[Utils::COLOR_NONE].height() * mSelectionOut[Utils::COLOR_NONE].width());
    fillPixelVector(pixVect, mSelectionOut[Utils::COLOR_NONE]);
    qint32 start = pixVect.size() - 1;

    if (!encodeToPixel(start, pixVect, 1, Utils::colorsObj(true, true, true), 12,
        1, mColors.rgb[0], 1, mColors.rgb[1], 1, mColors.rgb[2],
        1, mIsLookAhead, 1, mIsCompress, 1, mIsEncrypt)) {
            emit writeToConsole("[Raster] Not enough pixels to encode SETTINGS.\n");
            return false;
    }

    if (!encodeToPixel(start, pixVect, 1, mColors, 2, NUM_OF_SIZE_BITS, mMsg.size())) {
        emit writeToConsole("[Raster] Not enough pixels to encode LENGTH.\n");
        return false;
    }

    QVector<bool> secretBits;
    for (int i = 0; i < mMsg.size(); i++) {
        numToBits(mMsg[i], 8, secretBits);
    }

    if (mIsLookAhead) {
        if (!encodeLookAhead(start, mSelectionOut[Utils::COLOR_NONE], mKey, secretBits, pixVect)) {
            emit writeToConsole("[Raster] Not enough pixels to encode SECRET MESSAGE using LOOKAHEAD.\n");
            return false;
        }

        QStringList toConsole;
        toConsole << "[Raster] LookAhead algorithm RESULTS:\n";
        qint32 sum = 0;
        for (qint8 i = 0; i < 4; i++) {
            toConsole << " " << QString::number(mStats[i].total) << "x encoded in " << QString::number(i) << " bits.\n";
            sum += mStats[i].total;
        }
        toConsole << "[Raster] Number of meta pixels used is " << QString::number(mMetaStats) << ".\n";
        toConsole << "[Raster] Old message length would be " << QString::number(mMsg.size() * 8) << ".\n";
        toConsole << "[Raster] New message length is " << QString::number(sum * 2 + mMetaStats * 3 * 2) << "(" << QString::number((100 * (sum * 2 + mMetaStats * 3 * 2)) / (mMsg.size() * 8)) << "%).\n";

        emit writeToConsole(toConsole.join(""));

        return true;
    }

    if (!encodeToPixel(start, pixVect, 1, mColors, secretBits)) {
        emit writeToConsole("[Raster] Not enough pixels to encode SECRET MESSAGE.\n");
        return false;
    }

    return true;
}

bool Raster::Decode()
{
    QVector<QRgb *> pixVect(mSelectionIn[Utils::COLOR_NONE].height() * mSelectionIn[Utils::COLOR_NONE].width());
    fillPixelVector(pixVect, mSelectionIn[Utils::COLOR_NONE]);
    qint32 start = pixVect.size() - 1;

    setSeed(mSelectionIn[Utils::COLOR_NONE], mKey);

    bool r, g, b;
    bool decodedIsLookAhead;
    bool decodedIsCompress;
    bool decodedIsEncrypt;

    if (!decodeFromPixel(start, pixVect, 1, Utils::colorsObj(true, true, true), 12, 1, &r, 1, &g, 1, &b,
        1, &decodedIsLookAhead, 1, &decodedIsCompress, 1, &decodedIsEncrypt)) {
            emit writeToConsole("[Decode] Not enough pixels to decode SETTINGS.\n");
            return false;
    }

    mIsEncrypt = decodedIsEncrypt;

    Utils::colorsObj decodedColor(r, g, b);
    if (!decodedColor.numOfselected) {
        emit writeToConsole("[Decode] Number of selected colors is 0. Nowhere to decode from.\n");
        return false;
    }

    qint32 length;
    if (!decodeFromPixel(start, pixVect, 1, decodedColor, 2, NUM_OF_SIZE_BITS, &length)) {
        emit writeToConsole("[Decode] Not enough pixels to decode LENGTH.\n");
        return false;
    }

    QVector<bool> msgBoolVect;
    if (decodedIsLookAhead) {
        if (!decodeLookAhead(start, length * 8, msgBoolVect, pixVect)) {
            emit writeToConsole("[Decode] Not enough pixels to decode SECRET MESSAGE using LOOKAHEAD.\n");
            return false;
        }
    } else if (!decodeFromPixel(start, pixVect, 1, decodedColor, length * 8, msgBoolVect)) {
        emit writeToConsole("[Decode] Not enough pixels to decode SECRET MESSAGE.\n");
        return false;
    }

    QByteArray decoBytes;
    quint8 temp = 0x00;
    for (int i = 0; i < msgBoolVect.size(); i++) {
        if ((i % 8 == 0) && (i != 0)) {
            decoBytes.append(temp);
            temp = 0x00;
        }

        if (msgBoolVect[i]) {
            temp = temp | (0x01 << (i % 8));
        }
    }
    decoBytes.append(temp);

    emit sendMessage(decoBytes, decodedIsCompress, decodedIsEncrypt);

    //FIXME qchecksum

    return true;
}
