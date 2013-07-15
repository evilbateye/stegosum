#include "pointgenthread.h"
#define SUFFLEEOFFSET 2
#define NUM_OF_SIZE_BITS 24
#define NUM_OF_SETTINGS_BITS 6
#define NUM_OF_SETTINGS_PIXELS ((NUM_OF_SETTINGS_BITS % 3 == 0) ? (NUM_OF_SETTINGS_BITS / 3) : ((NUM_OF_SETTINGS_BITS / 3) + 1))

PointGenThread::PointGenThread(QWidget *parent) :
    QThread(parent), mEncode(true), mIsEncrypt(false)
{
}

void PointGenThread::run()
{
    if (mEncode) emit succes(Encode(mImg, mMsg, mKey));
    else emit succes(Decode(mImg, mKey));
}

void PointGenThread::setUp(const QImage & img, QByteArray & msg, quint16 key, bool encode, bool isCompress, bool isEncrypt, bool isLookAhead, bool colorR, bool colorG, bool colorB)
{
    mImg = img;
    mMsg = msg;
    mKey = key;

    mEncode = encode;
    mIsCompress = isCompress;
    mIsEncrypt = isEncrypt;
    mIsLookAhead = isLookAhead;

    mColors[0] = colorR;
    mColors[1] = colorG;
    mColors[2] = colorB;
}

const QImage & PointGenThread::getImg()
{
    return mImg;
}

void PointGenThread::numToBits(quint32 msgSize, quint32 shift, QVector<bool> & msgBoolVect)
{
    quint32 sizeMask;
    for (quint32 j = 0; j < shift; j++) {
        sizeMask = (0x00000001 << j);
        msgBoolVect.push_back((msgSize & sizeMask) == sizeMask);
    }
}

quint32 PointGenThread::bitsToNum(quint32 numBitsCount, QVector<bool> & msgBoolVect, quint32 shift)
{

    quint32 msgSize = 0x00000000;

    for (quint32 i = 0; i < numBitsCount; i++) {
        if (msgBoolVect[i + shift]) {
            msgSize = msgSize | (0x00000001 << i);
        }
    }

    return msgSize;
}

//Old and slow algorithm with many collisions (Already generated pseudorandom numbers).
void PointGenThread::shuffle(qint32 msgSizeB, QImage & image, quint16 key, QVector<QPoint> & pointsList)
{
    qint32 numImgPixels = image.height() * image.width() - SUFFLEEOFFSET;

    QRgb * first2pixels = (QRgb *) image.bits();

    quint32 seed = key ^ (first2pixels[0] + first2pixels[1]);

    qsrand(seed);

    while (pointsList.count() < msgSizeB && pointsList.count() < numImgPixels) {
        QPoint p;
        do {

            int pos = qrand() % numImgPixels + SUFFLEEOFFSET;
            p = QPoint(pos % image.width(), pos / image.width());

        } while (pointsList.contains(p));

        pointsList.append(p);

        if (mEncode) emit updateProgress(pointsList.size());

    }
}

//New algorithm using shuffle to generate only unique pseudorandom numbers (Much faster).
void PointGenThread::generatePixelPoints(qint32 msgSizeB, QList<QRgb *> & pointsList, QVector<QRgb *> & pixVect, quint8 offset)
{
//    emit setMaximum(msgSizeB);
//    int count = 0;

    quint32 start = pixVect.size() - 1 - offset;

    for (quint32 i = start; i > start - msgSizeB; i--) {

        quint32 pos = qrand() % (i + 1 - SUFFLEEOFFSET) + SUFFLEEOFFSET;

        pointsList.append(pixVect[pos]);

        //upgrade emission, slowing down coding
//        if (msgSizeB >= 100) {
//            count++;
//            if (count % (msgSizeB / 100) == 0)
//                emit updateProgress(msgSizeB / 100);
//        } else {
//            emit updateProgress(1);
//        }

        qSwap(pixVect[i], pixVect[pos]);
    }
}

//FIXME
bool PointGenThread::lookAheadEncodeAlgorithm(QVector<bool> & msgBVect, QVector<QRgb *> & pixVect, quint8 offset)
{
    quint32 oldMsgSize = msgBVect.size();
    resetLookAheadStatistics();

    quint32 start = pixVect.size() - 1 - offset;

    while (!msgBVect.isEmpty()) {

        if (pixVect.isEmpty()) {
            qDebug() << "[lookAhead] Not enough image pixels to encode the secret message.";
            qDebug() << "[lookAhead] There are " << mLAobj[3].total << " colors with 3 bits encoded.";
            qDebug() << "[lookAhead] There are " << mLAobj[2].total << " colors with 2 bits encoded.";
            qDebug() << "[lookAhead] There are " << mLAobj[1].total << " colors with 1 bits encoded.";
            qDebug() << "[lookAhead] There are " << mLAobj[0].total << " colors with 0 bits encoded.";
            return false;
        }

        quint32 pos = qrand() % (start + 1 - SUFFLEEOFFSET) + SUFFLEEOFFSET;

        QRgb * pixel = pixVect[pos];
        quint8 red = qRed(*pixel);
        quint8 green = qGreen(*pixel);
        quint8 blue = qBlue(*pixel);

        red = getEmbeddedLookAheadColor(msgBVect, red, PointGenThread::RED);
        green = getEmbeddedLookAheadColor(msgBVect, green, PointGenThread::GREEN);
        blue = getEmbeddedLookAheadColor(msgBVect, blue, PointGenThread::BLUE);

        *pixel = qRgba(red, green, blue, qAlpha(*pixel));

        qSwap(pixVect[start--], pixVect[pos]);
    }

    qDebug() << "[lookAhead] Message encoded succesfully.";
    qDebug() << "[lookAhead] There are " << mLAobj[3].total << " colors with 3 bits encoded.";
    qDebug() << "[lookAhead] There are " << mLAobj[2].total << " colors with 2 bits encoded.";
    qDebug() << "[lookAhead] There are " << mLAobj[1].total << " colors with 1 bits encoded.";
    qDebug() << "[lookAhead] There are " << mLAobj[0].total << " colors with 0 bits encoded.";

    quint32 total = mLAobj[0].total + mLAobj[1].total + mLAobj[2].total + mLAobj[3].total;
    qDebug() << "[lookAhead] Total of " << total << " colors are encoded.";
    qDebug() << "[lookAhead] Old message size is " << oldMsgSize << " bits (" << (((oldMsgSize % 3) == 0) ? oldMsgSize / 3 : oldMsgSize / 3 + 1) << " pixels with LSB encoding).";

    quint32 pixels = (((total % 3) == 0) ? total / 3 : total / 3 + 1);
    qDebug() << "[lookAhead] New message size is " << total * 2 <<  " bits. (" << pixels << " pixels with LSB (0. bit) and 1. bit encoding (would be " << pixels * 2 << " pixels with only LSB encoding))";
    qDebug() << "[lookAhead] New message is " << ((total * 2) * 100) / oldMsgSize << "% from the old one.";

    return true;
}

void PointGenThread::resetLookAheadStatistics()
{
    for (int i = 0; i < 4; i++) {
        mLAobj[i].bitsR = 0;
        mLAobj[i].bitsG = 0;
        mLAobj[i].bitsB = 0;
        mLAobj[i].total = 0;
    }
}

quint8 PointGenThread::getEmbeddedLookAheadColor(QVector<bool> & msgBoolVect, quint8 colorVal, Color colorType)
{
    QVector<bool> lookAheadVect;
    quint8 lookAhead = (colorVal & 0x1C) >> 2;
    numToBits(lookAhead, 3, lookAheadVect);

    quint8 ret = (colorVal & 0xFC);

    if (!msgBoolVect.isEmpty() && msgBoolVect[0] == lookAheadVect[0]) {

        msgBoolVect.pop_front();
        lookAheadVect.pop_front();

        if (!msgBoolVect.isEmpty() && msgBoolVect[0] == lookAheadVect[0]) {

            msgBoolVect.pop_front();
            lookAheadVect.pop_front();

            if (!msgBoolVect.isEmpty() && msgBoolVect[0] == lookAheadVect[0]) {

                msgBoolVect.pop_front();
                lookAheadVect.pop_front();

                lookAheadStatistics(BITS_ENCODED_3, colorType);
                return ret | 0x03;
            }

            lookAheadStatistics(BITS_ENCODED_2, colorType);
            return ret | 0x02;
        }

        lookAheadStatistics(BITS_ENCODED_1, colorType);
        return ret | 0x01;
    }

    lookAheadStatistics(BITS_ENCODED_0, colorType);
    return ret;
}

void PointGenThread::lookAheadStatistics(Bits bits, Color color)
{
    switch (color) {
        case RED: { mLAobj[bits].bitsR++; break;}
        case GREEN: { mLAobj[bits].bitsG++; break;}
        case BLUE: { mLAobj[bits].bitsB++; break;}
    }
    mLAobj[bits].total++;
}

quint32 PointGenThread::getMessageSizeInPixels(quint32 messageSizeInBits, quint8 numOfSelColors)
{
    if (!numOfSelColors) return 0;
    quint32 tmp = messageSizeInBits / numOfSelColors;
    if (messageSizeInBits % numOfSelColors != 0) tmp++;
    return tmp;
}

void PointGenThread::encodeSettings(QList<QRgb *> & pointsList)
{
    //Encode rgb info in first generated pixel.
    QRgb * Pixel = pointsList[0];
    quint8 red = qRed(*Pixel);
    quint8 green = qGreen(*Pixel);
    quint8 blue = qBlue(*Pixel);

    red = (red & 0xFE) | mColors[0];
    green = (green & 0xFE) | mColors[1];
    blue = (blue & 0xFE) | mColors[2];

    (*Pixel) = qRgba(red, green, blue, qAlpha(*Pixel));
    pointsList.pop_front();

    //Encode other info in second pixel.
    Pixel = pointsList[0];
    red = qRed(*Pixel);
    green = qGreen(*Pixel);
    blue = qBlue(*Pixel);

    red = (red & 0xFE) | mIsLookAhead;
    green = (green & 0xFE) | mIsCompress;
    blue = (blue & 0xFE) | mIsEncrypt;

    (*Pixel) = qRgba(red, green, blue, qAlpha(*Pixel));
    pointsList.pop_front();
}

void PointGenThread::setSeed(QImage & image, quint16 key)
{
    QRgb * first2pixels = (QRgb *) image.bits();
    quint32 seed = key ^ (first2pixels[0] + first2pixels[1]);
    qsrand(seed);
}

void PointGenThread::fillPixelVector(QVector<QRgb *> & pixVect, QImage & image)
{
    for (int i = 0; i < image.height(); i++) {
        for (int j = 0; j < image.width(); j++) {
            pixVect[i * image.width() + j] = (&reinterpret_cast<QRgb *>(image.scanLine(i))[j]);
        }
    }
}

bool PointGenThread::Encode(QImage & image, QByteArray & msgBytes, quint16 key)
{

    /*QImage simg("/home/evilbateye/Pictures/1318099542371.png");
    QByteArray msgBytes;
    QBuffer buffer(&msgBytes);
    buffer.open(QIODevice::WriteOnly);
    qDebug()<<simg.save(&buffer, "PNG");*/

    if (!getNumOfSelectedColors(mColors)) {
        qDebug() << "[Encode] No colors selected. Nowhere to encode.";
        return false;
    }

    image = image.convertToFormat(QImage::Format_ARGB32);

    quint32 msgSizeInBytes = msgBytes.size();

    QVector<bool> msgBoolVect;

    numToBits(msgSizeInBytes, NUM_OF_SIZE_BITS, msgBoolVect);

    for (int i = 0; i < msgBytes.size(); i++) {
        numToBits(msgBytes[i], 8, msgBoolVect);
    }

    int msgSizeInPix = PointGenThread::getMessageSizeInPixels(msgBoolVect.size(), getNumOfSelectedColors(mColors));
    if (msgSizeInPix + NUM_OF_SETTINGS_PIXELS > (image.height() * image.width() - SUFFLEEOFFSET)) {
        qDebug() << "[Encode] Not enough pixels to encode secret message.";
        return false;
    }

    setSeed(image, key);

    QVector<QRgb *> pixVect(image.height() * image.width());
    fillPixelVector(pixVect, image);

    QList<QRgb *> generatedPoints;
    generatePixelPoints(NUM_OF_SETTINGS_PIXELS, generatedPoints, pixVect);
    encodeSettings(generatedPoints);

    if (mIsLookAhead) return lookAheadEncodeAlgorithm(msgBoolVect, pixVect, NUM_OF_SETTINGS_PIXELS);

    generatePixelPoints(msgSizeInPix, generatedPoints, pixVect, NUM_OF_SETTINGS_PIXELS);

    int i = 0;
    foreach (QRgb * pixel, generatedPoints) {

        quint8 updatedColors[4] = {qRed(*pixel), qGreen(*pixel), qBlue(*pixel), qAlpha(*pixel)};

        for (int j = 0; j < 3; j++) {
            if (mColors[j] && i < msgBoolVect.size()) {
                bool b = msgBoolVect[i++];
                updatedColors[j] = (updatedColors[j] & 0xFE) | b;
            }
        }

        *pixel = qRgba(updatedColors[0], updatedColors[1], updatedColors[2], updatedColors[3]);
    }

    return true;
}

void PointGenThread::decodeSettings(QList<QRgb *> & points, bool * settings)
{
    //Decode color setings from 1. point
    QRgb * pixel = points[0];
    quint8 red = qRed(*pixel);
    quint8 green = qGreen(*pixel);
    quint8 blue = qBlue(*pixel);

    settings[0] = red & 0x01;
    settings[1] = green & 0x01;
    settings[2] = blue & 0x01;
    points.pop_front();

    //Decode other settings from 2. point
    pixel = points[0];
    red = qRed(*pixel);
    green = qGreen(*pixel);
    blue = qBlue(*pixel);

    settings[3] = red & 0x01;
    settings[4] = green & 0x01;
    settings[5] = blue & 0x01;
    points.pop_front();

    mIsEncrypt = settings[5];
}

quint8 PointGenThread::getNumOfSelectedColors(bool * colors)
{
    quint8 tmp = 0;
    if (colors[0]) tmp++;
    if (colors[1]) tmp++;
    if (colors[2]) tmp++;
    return tmp;
}

bool PointGenThread::Decode(QImage & image, quint16 key)
{
    image = image.convertToFormat(QImage::Format_ARGB32);
    QVector<QRgb *> pixVect(image.height() * image.width());
    fillPixelVector(pixVect, image);

    setSeed(image, key);

    QList<QRgb *> generatedPoints;
    generatePixelPoints(NUM_OF_SETTINGS_PIXELS, generatedPoints, pixVect);

    bool settings[NUM_OF_SETTINGS_BITS];
    decodeSettings(generatedPoints, settings);

    if (!getNumOfSelectedColors(settings)) {
        qDebug() << "[Decode] Number of decoded selected colors is 0. There is nothing to decode from image pixels.";
        return false;
    }

    //FIXME
    if (settings[3]) {;} //lookahead decode + return

    int numOfPtsToDecodeMsgLen = PointGenThread::getMessageSizeInPixels(NUM_OF_SIZE_BITS, getNumOfSelectedColors(settings));
    generatePixelPoints(numOfPtsToDecodeMsgLen, generatedPoints, pixVect, NUM_OF_SETTINGS_PIXELS);

    QVector<bool> msgBoolVect;
    foreach (QRgb * pixel, generatedPoints) {

        quint8 decodedColors[4] = {qRed(*pixel), qGreen(*pixel), qBlue(*pixel), qAlpha(*pixel)};

        for (int j = 0; j < 3; j++) {
            if (settings[j]) {
                msgBoolVect.push_back(decodedColors[j] & 0x01);
            }
        }
    }
    generatedPoints.clear();

    quint32 msgLenInBytes = bitsToNum(NUM_OF_SIZE_BITS, msgBoolVect);
    msgBoolVect.clear();


    quint32 numOfPtsToDecodeMsg = PointGenThread::getMessageSizeInPixels(msgLenInBytes * 8, getNumOfSelectedColors(settings));

    if (numOfPtsToDecodeMsg + numOfPtsToDecodeMsgLen + NUM_OF_SETTINGS_PIXELS > image.height() * image.width() - SUFFLEEOFFSET) {
        qDebug() << "[Decode] Decoded length of the secret message is too big. Cannot decode the secret message.";
        return false;
    }

    generatePixelPoints(numOfPtsToDecodeMsg, generatedPoints, pixVect, NUM_OF_SETTINGS_PIXELS + numOfPtsToDecodeMsgLen);

    foreach (QRgb * pixel, generatedPoints) {

        quint8 decodedColors[4] = {qRed(*pixel), qGreen(*pixel), qBlue(*pixel), qAlpha(*pixel)};

        for (int j = 0; j < 3; j++) {
            if (settings[j]) {
                msgBoolVect.push_back(decodedColors[j] & 0x01);
            }
        }
    }

    quint8 numCols = getNumOfSelectedColors(settings);
    if ((msgLenInBytes * 8) % numCols != 0) {
        for (quint8 i = numCols - 1; i > 0; i--) {
            msgBoolVect.pop_back();
            if ((msgLenInBytes * 8) % numCols == i) break;
        }
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

    emit sendMessage(decoBytes, settings[4], settings[5]);

    //FIXME if encoded msg size != msg we extracted --> return false, use qchecksum
    return true;
}
