#include "pointgenthread.h"
#define SUFFLEEOFFSET 2
#define SIZE_ENCODE_OFFSET 24
#define SETTINGS_OFFSET 3
#define RED_SPECTRUM 1
#define GREEN_SPECTRUM 2
#define BLUE_SPECTRUM 3

PointGenThread::PointGenThread(QWidget *parent) :
    QThread(parent), mEncode(true), mIsEncrypt(false), mIsEncodeMax(false)
{
}

void PointGenThread::run()
{
    if (mEncode) emit succes(Encode(mImg, mMsg, mKey));
    else emit succes(Decode(mImg, mKey));
}

void PointGenThread::setUp(const QImage & img, QByteArray & msg, quint16 key, bool encode, bool isCompress, bool isEncrypt, bool isEncodeMax, Color color)
{
    mImg = img;
    mMsg = msg;
    mKey = key;

    mEncode = encode;
    mIsCompress = isCompress;
    mIsEncrypt = isEncrypt;
    mIsEncodeMax = isEncodeMax;
    mColor = color;
}

const QImage & PointGenThread::getImg()
{
    return mImg;
}

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

void PointGenThread::shuffle2(qint32 msgSizeB, QImage & image, QList<QRgb *> & pointsList, QVector<QRgb *> & pixVect, quint8 offset)
{

    qint32 numImgPixels = image.height() * image.width();

    emit setMaximum(msgSizeB);

    int count = 0;

    for (int i = numImgPixels - 1 - offset; i > numImgPixels - 1 - msgSizeB - offset; i--)
    {
        int pos = qrand() % (i + 1 - SUFFLEEOFFSET) + SUFFLEEOFFSET;

        pointsList.append(pixVect[pos]);

        //upgrade emission, slowing down coding

        if (msgSizeB >= 100)
        {
            count++;
            if (count % (msgSizeB / 100) == 0)
                emit updateProgress(msgSizeB / 100);
        }
        else
        {
            emit updateProgress(1);
        }

        qSwap(pixVect[i], pixVect[pos]);
    }

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

bool PointGenThread::Encode(QImage & image, QByteArray & msgBytes, quint16 key)
{
    //FIXME

    /*QImage simg("/home/evilbateye/Pictures/1318099542371.png");
    QByteArray msgBytes;
    QBuffer buffer(&msgBytes);
    buffer.open(QIODevice::WriteOnly);
    qDebug()<<simg.save(&buffer, "PNG");*/

    //

    image = image.convertToFormat(QImage::Format_ARGB32);

    quint32 msgSize = msgBytes.size();

    QVector<bool> msgBoolVect;
    msgBoolVect.push_back(mIsCompress);
    msgBoolVect.push_back(mIsEncrypt);
    msgBoolVect.push_back(mIsEncodeMax);

    numToBits(msgSize, SIZE_ENCODE_OFFSET, msgBoolVect);

    for (int i = 0; i < msgBytes.size(); i++) {
        numToBits(msgBytes[i], 8, msgBoolVect);
    }


    int msgSizeInPix = msgBoolVect.size() - 2 + 1; //-2 because settings are in one pixel +1 pre colors
    if (mIsEncodeMax)
    {
        msgSizeInPix = msgBoolVect.size() / 3;
        if (msgBoolVect.size() % 3 != 0)
            msgSizeInPix++;
    }

    qint32 numImgPixels = image.height() * image.width();
    if (msgSizeInPix > (numImgPixels - SUFFLEEOFFSET)) { return false; }

    QRgb * first2pixels = (QRgb *) image.bits();
    quint32 seed = key ^ (first2pixels[0] + first2pixels[1]);
    qsrand(seed);

    QList<QRgb *> pointsList;
    QVector<QRgb *> pixVect(numImgPixels);

    for (int i = 0; i < image.height(); i++)
    {
        for (int j = 0; j < image.width(); j++)
        {
            pixVect[i * image.width() + j] = (&reinterpret_cast<QRgb *>(image.scanLine(i))[j]);
        }
    }

    shuffle2(msgSizeInPix, image, pointsList, pixVect);

    if (mIsEncodeMax) {

        for (int i = 0; i < pointsList.size(); i++) {

            QRgb * pixel = pointsList[i];

            QRgb tmpPixel = (* pixel);

            quint8 red = qRed(tmpPixel);

            quint8 green = qGreen(tmpPixel);

            quint8 blue = qBlue(tmpPixel);


            red = (red & 0xFE);
            if (i * 3 < msgBoolVect.size())
                if (msgBoolVect[i * 3])
                {
                    red = red | 0x01;
                }


            green = (green & 0xFE);
            if (i * 3 + 1 < msgBoolVect.size())
                if (msgBoolVect[i * 3 + 1])
                {
                    green = green | 0x01;
                }


            blue = (blue & 0xFE);
            if (i * 3 + 2 < msgBoolVect.size())
                if (msgBoolVect[i * 3 + 2])
                {
                    blue = blue | 0x01;
                }

            (* pixel) = qRgba(red, green, blue, qAlpha(tmpPixel));
        }

    } else {

        QRgb * settiPix = pointsList[0];
        QRgb tmpSettiPix = (* settiPix);
        quint8 settiRed = qRed(tmpSettiPix);
        quint8 settiGreen = qGreen(tmpSettiPix);
        quint8 settiBlue = qBlue(tmpSettiPix);

        settiRed = (settiRed & 0xFE);
        settiGreen = (settiGreen & 0xFE);
        settiBlue = (settiBlue & 0xFE);

        if (msgBoolVect[0]) settiRed = settiRed | 0x01;
        if (msgBoolVect[1]) settiGreen = settiGreen | 0x01;
        if (msgBoolVect[2]) settiBlue = settiBlue | 0x01;

        (* settiPix) = qRgba(settiRed, settiGreen, settiBlue, qAlpha(tmpSettiPix));

        pointsList.pop_front();
        msgBoolVect.erase(msgBoolVect.begin(), msgBoolVect.begin() + SETTINGS_OFFSET);

        settiPix = pointsList[0];
        tmpSettiPix = (* settiPix);
        settiRed = qRed(tmpSettiPix);
        settiGreen = qGreen(tmpSettiPix);
        settiBlue = qBlue(tmpSettiPix);

        settiRed = (settiRed & 0xFE);
        settiGreen = (settiGreen & 0xFE);
        settiBlue = (settiBlue & 0xFE);

        if (mColor == RED) settiRed = settiRed | 0x01;
        if (mColor == GREEN) settiGreen = settiGreen | 0x01;
        if (mColor == BLUE) settiBlue = settiBlue | 0x01;
        if (mColor == ALL) {
            //DO NOTHING
        }

        (* settiPix) = qRgba(settiRed, settiGreen, settiBlue, qAlpha(tmpSettiPix));
        pointsList.pop_front();

        for (int i = 0; i < pointsList.size(); i++) {

            QRgb * pixel = pointsList[i];

            QRgb tmpPixel = (* pixel);

            quint8 red = qRed(tmpPixel);

            quint8 green = qGreen(tmpPixel);

            quint8 blue = qBlue(tmpPixel);

            switch (mColor) {
                case RED:
                    red = (red & 0xFE);
                    if (msgBoolVect[i]) red = red | 0x01;
                    break;
                case GREEN:
                    green = (green & 0xFE);
                    if (msgBoolVect[i]) green = green | 0x01;
                    break;
                case BLUE:
                    blue = (blue & 0xFE);
                    if (msgBoolVect[i]) blue = blue | 0x01;
                    break;
                case ALL:
                    switch (i % 3) {
                        case 0:
                            red = (red & 0xFE);
                            if (msgBoolVect[i]) red = red | 0x01;
                            break;
                        case 1:
                            green = (green & 0xFE);
                            if (msgBoolVect[i]) green = green | 0x01;
                            break;
                        case 2:
                            blue = (blue & 0xFE);
                            if (msgBoolVect[i]) blue = blue | 0x01;
                            break;
                    }
                    break;

            }

            (* pixel) = qRgba(red, green, blue, qAlpha(tmpPixel));
        }
    }

//
/*
    for (int i = 0; i < image.height(); i++)
    {
        for (int j = 0; j < image.width(); j++)
        {
            QRgb * pixel = (&reinterpret_cast<QRgb *>(image.scanLine(i))[j]);
            QRgb tmpPixel = (* pixel);
            quint8 red = qRed(tmpPixel);
            quint8 green = qGreen(tmpPixel);
            quint8 blue = qBlue(tmpPixel);
            red = (red & 0x00);
            green = (green & 0x01);
            blue = (blue & 0x00);
            //red = (red << 7);
            green = (green << 7);
            //blue = (blue << 7);
            (* pixel) = qRgba(red, green, blue, qAlpha(tmpPixel));
        }
    }
*/
//
    return true;
}

bool PointGenThread::Decode(QImage & image, quint16 key)
{
    image = image.convertToFormat(QImage::Format_ARGB32);
    QVector<bool> msgBoolVect;
    quint8 mask = 0x01;
    quint8 temp = 0x00;
    bool isRED;
    bool isGREEN;
    bool isBLUE;
    bool isAll;

    qint32 numImgPixels = image.height() * image.width();

    QList<QRgb *> pointsList;
    QVector<QRgb *> pixVect(numImgPixels);

    for (int i = 0; i < image.height(); i++)
    {
        for (int j = 0; j < image.width(); j++)
        {
            pixVect[i * image.width() + j] = (&reinterpret_cast<QRgb *>(image.scanLine(i))[j]);
        }
    }

    QRgb * first2pixels = (QRgb *) image.bits();

    quint32 seed = key ^ (first2pixels[0] + first2pixels[1]);

    qsrand(seed);

    shuffle2(1, image, pointsList, pixVect);

    QRgb * settiPix = pointsList[0];
    QRgb tmpSettiPix = (* settiPix);
    quint8 settiRed = qRed(tmpSettiPix);
    quint8 settiGreen = qGreen(tmpSettiPix);
    quint8 settiBlue = qBlue(tmpSettiPix);
    bool isCompress = (settiRed & 0x01) == 0x01;
    bool isEncrypt = (settiGreen & 0x01) == 0x01;
    mIsEncrypt = isEncrypt;

    bool is3Channel = (settiBlue & 0x01) == 0x01;

    pointsList.pop_front();

    if (is3Channel) {

        shuffle2(SIZE_ENCODE_OFFSET / 3, image, pointsList, pixVect, 1);

        for (int i = 0; i < SIZE_ENCODE_OFFSET / 3; i++) {

            QRgb * pixel = pointsList[i];

            QRgb tmpPixel = (* pixel);

            quint8 red = qRed(tmpPixel);

            quint8 green = qGreen(tmpPixel);

            quint8 blue = qBlue(tmpPixel);

            msgBoolVect.push_back((red & mask) == mask);

            msgBoolVect.push_back((green & mask) == mask);

            msgBoolVect.push_back((blue & mask) == mask);

        }

    } else {

        shuffle2(1, image, pointsList, pixVect, 1);

        settiPix = pointsList[0];
        tmpSettiPix = (* settiPix);
        settiRed = qRed(tmpSettiPix);
        settiGreen = qGreen(tmpSettiPix);
        settiBlue = qBlue(tmpSettiPix);
        isRED = (settiRed & 0x01) == 0x01;
        isGREEN = (settiGreen & 0x01) == 0x01;
        isBLUE = (settiBlue & 0x01) == 0x01;
        isAll = (!isRED && !isGREEN && !isBLUE);
        pointsList.pop_front();


        shuffle2(SIZE_ENCODE_OFFSET, image, pointsList, pixVect, 1 + 1);

        for (int i = 0; i < SIZE_ENCODE_OFFSET; i++) {

            QRgb * pixel = pointsList[i];

            QRgb tmpPixel = (* pixel);

            quint8 red = qRed(tmpPixel);

            quint8 green = qGreen(tmpPixel);

            quint8 blue = qBlue(tmpPixel);

            if (isRED) msgBoolVect.push_back((red & mask) == mask); else
            if (isGREEN) msgBoolVect.push_back((green & mask) == mask); else
            if (isBLUE) msgBoolVect.push_back((blue & mask) == mask); else
            if (isAll) {
                switch (i % 3) {
                    case 0:
                        msgBoolVect.push_back((red & mask) == mask);
                        break;
                    case 1:
                        msgBoolVect.push_back((green & mask) == mask);
                        break;
                    case 2:
                        msgBoolVect.push_back((blue & mask) == mask);
                        break;
                }
            }

        }
    }

    quint32 msgSize = bitsToNum(SIZE_ENCODE_OFFSET, msgBoolVect);
    quint32 numPointsToGenerate = ((msgSize * 8) % 3 == 0) ? (msgSize * 8) / 3 : (msgSize * 8) / 3 + 1;
    qint32 msgBoolSize = msgSize * 8 + SIZE_ENCODE_OFFSET + 1 + 1; //+1 because settings and colors are in 1 pixel
    if (is3Channel) msgBoolSize = (msgSize * 8 + SIZE_ENCODE_OFFSET + SETTINGS_OFFSET) / 3 + 1;

    if (msgBoolSize > numImgPixels - SUFFLEEOFFSET) { return false; }

    msgBoolVect.clear();
    pointsList.clear();

    if (is3Channel) {

        shuffle2(numPointsToGenerate, image, pointsList, pixVect, 1 + SIZE_ENCODE_OFFSET / 3);

        for (quint32 i = 0; i < numPointsToGenerate; i++) {

            QRgb * pixel = pointsList[i];

            QRgb tmpPixel = (* pixel);

            quint8 red = qRed(tmpPixel);

            quint8 green = qGreen(tmpPixel);

            quint8 blue = qBlue(tmpPixel);

            msgBoolVect.push_back((red & mask) == mask);

            if (i != pointsList.size() - 1)
                msgBoolVect.push_back((green & mask) == mask);
            else if ((msgSize * 8) % 3 != 1)
                msgBoolVect.push_back((green & mask) == mask);

            if (i != pointsList.size() - 1)
                msgBoolVect.push_back((blue & mask) == mask);
            else if ((msgSize * 8) % 3 == 0)
                msgBoolVect.push_back((blue & mask) == mask);

        }

    } else {

        shuffle2(msgSize * 8, image, pointsList, pixVect, SIZE_ENCODE_OFFSET + 1 + 1);

        for (quint32 i = 0; i < msgSize * 8; i++) {

            QRgb * pixel = pointsList[i];

            QRgb tmpPixel = (* pixel);

            quint8 red = qRed(tmpPixel);

            quint8 green = qGreen(tmpPixel);

            quint8 blue = qBlue(tmpPixel);

            if (isRED) msgBoolVect.push_back((red & mask) == mask); else
            if (isGREEN) msgBoolVect.push_back((green & mask) == mask); else
            if (isBLUE) msgBoolVect.push_back((blue & mask) == mask); else
            if (isAll) {
                switch (i % 3) {
                    case 0:
                        msgBoolVect.push_back((red & mask) == mask);
                        break;
                    case 1:
                        msgBoolVect.push_back((green & mask) == mask);
                        break;
                    case 2:
                        msgBoolVect.push_back((blue & mask) == mask);
                        break;
                }
            }
        }
    }

    QByteArray decoBytes;
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

    emit sendMessage(decoBytes, isCompress, isEncrypt);

    //FIXME if encoded msg size != msg we extracted --> return false, use qchecksum
    return true;
}
