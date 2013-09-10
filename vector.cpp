#include <QLineF>

#include "vector.h"
#include <QtSvg>
#include "utils.hpp"

#define NUM_MSG_LEN_NUMS 8
#define ANGLE_PREC 1
//#define FLOATING_POINT_POS 9
//#define SHOW_POINTS_W_AND_H 1

#include <iostream>
void debugMessage(QString msg) {
    while (!msg.isEmpty()) {
        std::cout << msg.left(8).toStdString() << " ";
        msg.remove(0, 8);
    }
    std::cout << std::endl;
}

Vector::Vector() : mIsDebug(true)
{
}

qreal Vector::totalMessageLength(QString msg, int fpppos) {
    qreal length = 0;

    while (!msg.isEmpty()) {
        length += Stegosum::streamToReal(msg.left(8), fpppos);
        msg.remove(0, 8);
    }

    return length;
}

bool Vector::nextPointSecret(QDomNodeList & nodes, QString & msg, int & polylineStart, int & lineStart, qreal & c, int fppos) {

    if (polylineStart == nodes.size()) return true;

    QLineF line;
    line.setP1(QPointF(0, 0));

    QLineF next;
    next.setP1(QPointF(0, 0));

    bool isBreak = false;

    for (; polylineStart < nodes.size(); polylineStart++) {

        QStringList in = nodes.at(polylineStart).toElement().attribute("d").split(" ");
        if (in.contains("s", Qt::CaseInsensitive)) continue;
        if (in.contains("c", Qt::CaseInsensitive)) continue;
        if (in.contains("l", Qt::CaseInsensitive)) continue;

        for (; lineStart < in.size() - 1; lineStart++) {

            line.setP2(QPointF(in.at(lineStart).split(",").first().toDouble(), in.at(lineStart).split(",").last().toDouble()));
            next.setP2(QPointF(in.at(lineStart + 1).split(",").first().toDouble(), in.at(lineStart + 1).split(",").last().toDouble()));

            if (linesNotParallel(line, next)) {
                c = QString::number(c + QString::number(line.length(), 'g', 8).toDouble(), 'g', 8).toDouble();
                continue;
            }

            QString digits = Stegosum::digitStream(QString::number(line.length(), 'g', 8).toDouble() + c, fppos);
            msg.append(digits);
            isBreak = true;
            lineStart++;
            c = 0;
            break;
        }

        if (lineStart == in.size() - 1) {

            lineStart = 2;
            if (in.size() == 3) next.setP2(QPointF(in.at(2).split(",").first().toDouble(), in.at(2).split(",").last().toDouble()));
            c = QString::number(c + QString::number(next.length(), 'g', 8).toDouble(), 'g', 8).toDouble();

            if (isBreak) {
                polylineStart++;
                break;
            }

        } else break;
    }

    return false;
}

bool Vector::Encode() {

    // A little hack to ignore the encrypt checkbox.
    // In raster images we can choose to use a password
    // to randomly spread out the message in LSB bits,
    // but not use it to encrypt the message. In vector
    // images there is no spread out.
    if (!mPassword.isEmpty()) {
        mIsEncrypt = true;
        mMsg = Utils::encrypt(mMsg, mPassword);
    }

    QDomDocument doc("svgFile");

    QFile file(mImageName);

    //FIXME1
    if (mIsDebug) {
        file.setFileName("/home/evilbateye/develop/CD/stegosum-build-desktop-Qt_4_8_3_in_PATH__System__Release/drawing.svg");
        mMsg = "012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
    }

    if (!file.open(QIODevice::ReadOnly)) {
        emit writeToConsole("[Vector] Cannot open input vector file for encoding.\n");
        return false;
    }

    if (!doc.setContent(&file)) {
        emit writeToConsole("[Vector] Cannot parse input vector file for encoding.\n");
        file.close();
        return false;
    }
    file.close();

    QDomNodeList nl = doc.elementsByTagName("path");

    quint8 byte;
    QString msgBytesAsString;

    msgBytesAsString.append(QString(NUM_MSG_LEN_NUMS - QString::number(mMsg.size()).size(), '0') + QString::number(mMsg.size()));

    if (mIsDebug) qDebug() << "mMsg.size() = " << mMsg.size();

    int settings = 0;
    settings = mIsCompress;
    settings = (mIsEncrypt << 1) | settings;
    msgBytesAsString.append(QString::number(settings));

    for (int i = 0; i < mMsg.size(); i++) {
        byte = mMsg[i];
        QString sbyte = QString::number(byte);
        QString append = QString(3 - sbyte.size(), '0') + sbyte;
        msgBytesAsString.append(append);
    }

    //FIXME1
    if (mIsDebug) debugMessage(msgBytesAsString);

    if (mIsFPPosMax) {

        // Get the total sum of all lines lengths.
        qreal totalLength = 0;
        for (qint32 i = 0; i < nl.size(); i++) {
            QStringList in = nl.at(i).toElement().attribute("d").split(" ");
            if (in.contains("s", Qt::CaseInsensitive)) continue;
            if (in.contains("c", Qt::CaseInsensitive)) continue;
            if (in.contains("l", Qt::CaseInsensitive)) continue;

            totalLength += polyLineLength(in);
        }

        // Get the max floating point position,
        // that still enables the message to be encoded.
        int pointPos = 9;
        qreal totalMsgLen = totalMessageLength(msgBytesAsString, pointPos);
        while (totalMsgLen > totalLength) {
            if (--pointPos < 0) {
                emit writeToConsole("[Vector] Not enough length in lines to encode secret message.\n");
                return false;
            }
            totalMsgLen /= 10;
        }

        mFPPos = pointPos;
    }

    //FIXME1
    if (mIsDebug) qDebug() << "mFPPos = " << mFPPos;

    bool firstTime = true;

    for (qint32 i = 0; i < nl.size(); i++) {

        QStringList in = nl.at(i).toElement().attribute("d").split(" ");

        if (in.contains("s", Qt::CaseInsensitive)) continue;
        if (in.contains("c", Qt::CaseInsensitive)) continue;
        if (in.contains("l", Qt::CaseInsensitive)) continue;

        // First polyline's first point adjustment.
        if (firstTime) {
            // Initialize pseudorandom number generator
            // using user's password and first point's Y coord.
            qsrand(mKey ^ digitStream(in.at(1).split(",").last().toDouble(), 9).toInt());

            // Randomize floating point position info.
            QString numberSelector = QString("0123456789").repeated(2);
            int random = qrand() % 10;
            int fppos = numberSelector.at(random + mFPPos).digitValue();

            // Encode the floating point position info
            // into X coord's last digit
            // (X coord of the first point).
            in[1] = setDigitAt(in.at(1).split(",").first(), fppos) + "," + in.at(1).split(",").last();

            firstTime = false;
        }

        // The polyline musn't have parallel sublines
        // in it before we can start encoding.
        preparePolyLine(in);
        QStringList out(in);

        // Algorithm branches based on relative
        // or absolute input coordinates.
        bool m = in.at(0) == "m";

        // Output coordinates are always relative.
        out[0] = out[0].toLower();

        int encoded = 0;
        QLineF line;
        line.setP1(QPointF(in.at(1).split(",").first().toDouble(), in.at(1).split(",").last().toDouble()));

        for (int j = 2; j < in.size(); j++) {

            line.setP2(QPointF(in.at(j).split(",").first().toDouble(), in.at(j).split(",").last().toDouble()));
            if (m)
                line.setP2(line.p1() + line.p2());
            else
                out[j + encoded] = QString::number(line.dx(), 'g', 8) + "," + QString::number(line.dy(), 'g', 8);

            qreal a, b, c, asum, bsum, csum;
            asum = bsum = csum = 0;

            while (!msgBytesAsString.isEmpty()) {

                // c = msgBytesAsString.left(8).insert(FLOATING_POINT_POS, '.').toDouble();
                c = Stegosum::streamToReal(msgBytesAsString.left(8), mFPPos);

                csum += c;

                if (csum > line.length()) {
                    // Msg doesn't fit to the end of this line.
                    // Adjust new relative coords of this lines end point,
                    // based on last encoded point.

                    a = QString::number(line.dx() - asum, 'g', 8).toDouble();
                    b = QString::number(line.dy() - bsum, 'g', 8).toDouble();
                    out[j + encoded] = QString::number(a, 'g', 8) + "," + QString::number(b, 'g', 8);

                    // We  encoded only a part of the msg distance,
                    // and we still need to encode the rest after this end point.
                    msgBytesAsString = msgBytesAsString.mid(8);

                    qreal sqrt = QString::number(qSqrt(qPow(a, 2) + qPow(b, 2)), 'g', 8).toDouble();

                    QString digits;
                    digits = Stegosum::digitStream(c - sqrt, mFPPos);

                    msgBytesAsString.prepend(digits);

                    // Deprecated:
                    // Point before j + encoded has y coord odd,
                    // j + encoded point doesn't have secret message.
                    // out[j + encoded - 1] = out[j + encoded - 1].split(",").first() + "," + setEven(out[j + encoded - 1].split(",").last(), false);

                    break;
                }

                msgBytesAsString = msgBytesAsString.mid(8);

                a = (c * line.dx()) / QString::number(line.length(), 'g', 8).toDouble();
                b = (c * line.dy()) / QString::number(line.length(), 'g', 8).toDouble();

                QString A = QString::number(a, 'g', 8);
                QString B = QString::number(b, 'g', 8);

                // Precision correction.
                qreal C = QLineF(0, 0, A.toDouble(), B.toDouble()).length();
                int dif = Stegosum::digitStream(c, mFPPos).toInt() - Stegosum::digitStream(C, mFPPos).toInt();
                if (dif) {

                    QString tmpA(A), tmpB(B);
                    if (qFabs(A.toDouble()) < qFabs(B.toDouble()))
                        tmpA = addToReal(A.toDouble(), dif, mFPPos);
                    else
                        tmpB = addToReal(B.toDouble(), dif, mFPPos);

                    C = QLineF(0, 0, tmpA.toDouble(), tmpB.toDouble()).length();
                    dif = Stegosum::digitStream(c, mFPPos).toInt() - Stegosum::digitStream(C, mFPPos).toInt();
                    if (dif) {
                        if (qFabs(A.toDouble()) > qFabs(B.toDouble()))
                            A = addToReal(A.toDouble(), dif, mFPPos);
                        else
                            B = addToReal(B.toDouble(), dif, mFPPos);
                    } else {
                        A = tmpA;
                        B = tmpB;
                    }
                }

                if (mIsDebug){
                    C = QLineF(0, 0, A.toDouble(), B.toDouble()).length();
                    dif = Stegosum::digitStream(c, mFPPos).toInt() - Stegosum::digitStream(C, mFPPos).toInt();

                    if (dif) {
                        qDebug() << "sumtin wong";
                    }
                }

                out.insert(j + encoded++, A + "," + B);


                asum = QString::number(asum + A.toDouble(), 'g', 8).toDouble();
                bsum = QString::number(bsum + B.toDouble(), 'g', 8).toDouble();

                // Deprecated:
                // If last number of y coordinate is even,
                // next point has secret message.
                // bstr = setEven(b, true);
                // out.insert(j + encoded++, QString::number(a, 'g', 8) + "," + bstr);

                if (msgBytesAsString.isEmpty()) {
                    // Msg has been successfuly encoded,
                    // but we still need to adjust this lines end point's coords.

                    // Deprecated:
                    // c = line.length() - csum;
                    // qreal a2 = ((c * line.dx()) / line.length());
                    // qreal b2 = ((c * line.dy()) / line.length());

                    a = QString::number(line.dx() - asum, 'g', 8).toDouble();
                    b = QString::number(line.dy() - bsum, 'g', 8).toDouble();
                    out[j + encoded] = QString::number(a, 'g', 8) + "," + QString::number(b, 'g', 8);

                    // Deprecated:
                    // Point before j + encoded has y coord odd,
                    // j + encoded point doesn't have secret message.
                    // out[j + encoded - 1] = out[j + encoded - 1].split(",").first() + "," + setEven(out[j + encoded - 1].split(",").last(), false);

                }

            }

            line.setP1(line.p2());
        }

        nl.at(i).toElement().setAttribute("d", out.join(" "));

        if (msgBytesAsString.isEmpty()) break;
    }

    if (!msgBytesAsString.isEmpty()) {
        emit writeToConsole("[Vector] Not enough lines to encode secret message.\n");
        return false;
    }

    mXml = doc.toByteArray();

    QBuffer buff(&mXml);
    mImage.load(&buff, "svg");

    //FIXME1
    if (mIsDebug){
        QFile of("/home/evilbateye/develop/CD/stegosum-build-desktop-Qt_4_8_3_in_PATH__System__Release/drawing_out.svg");
        of.open(QFile::WriteOnly);
        QTextStream ts(&of);
        ts << mXml;
        of.close();
        Decode();
    }

    return true;
}

bool Vector::Decode() {

    QDomDocument doc("svgFile");

    //FIXME1
    QFile file(mImageName);
    if (mIsDebug) file.setFileName("/home/evilbateye/develop/CD/stegosum-build-desktop-Qt_4_8_3_in_PATH__System__Release/drawing_out.svg");

    if (!file.open(QIODevice::ReadOnly)) {
        emit writeToConsole("[Vector] Cannot open input vector file for decoding.\n");
        return false;
    }

    if (!doc.setContent(&file)) {
        emit writeToConsole("[Vector] Cannot parse input vector file for decoding.\n");
        file.close();
        return false;
    }
    file.close();

    QDomNodeList nl = doc.elementsByTagName("path");

    int fppos;
    for (qint32 i = 0; i < nl.size(); i++) {

        QStringList in = nl.at(i).toElement().attribute("d").split(" ");

        if (in.contains("s", Qt::CaseInsensitive)) continue;
        if (in.contains("c", Qt::CaseInsensitive)) continue;
        if (in.contains("l", Qt::CaseInsensitive)) continue;

        qsrand(mKey ^ digitStream(in.at(1).split(",").last().toDouble(), 9).toInt());
        QString numberSelector = QString("0123456789").repeated(2);
        int random = qrand() % 10;

        fppos = digitAt(in.at(1).split(",").first());
        fppos = numberSelector.indexOf(QString::number(fppos), random) - random;

        break;
    }

    //FIXME1
    if (mIsDebug) qDebug() << "fppos = " << fppos;

    QString msgBytesAsString;
    int firstPolyL = 0;
    int firstL = 2;
    qreal prevDistance = 0.0;

    nextPointSecret(nl, msgBytesAsString, firstPolyL, firstL, prevDistance, fppos);
    int msgLen = msgBytesAsString.left(8).toInt();

    if (mIsDebug) qDebug() << "msgLen = " << msgLen;

    if (!msgLen) {
        emit writeToConsole("[Vector] Decoded length of the secret message is 0. Are you sure this image contains a message?\n");
        return false;
    }

    msgBytesAsString.clear();

    nextPointSecret(nl, msgBytesAsString, firstPolyL, firstL, prevDistance, fppos);
    int settings = msgBytesAsString.at(0).digitValue();
    bool isCompress = settings & 1;
    bool isEncrypt = (settings >> 1) & 1;
    msgBytesAsString.remove(0, 1);

    while (msgBytesAsString.size() / 3 < msgLen) {

        if (nextPointSecret(nl, msgBytesAsString, firstPolyL, firstL, prevDistance, fppos)) {
            emit writeToConsole("[Vector] Decoding failed. Decoded length of the secret message is too big. Image doesn't have a secret message, maybe?\n");
            return false;
        }
    }

//    QDomNodeList nl = doc.elementsByTagName("path");

//    QString msgBytesAsString;

//    int msgLen;

//    int start = 2;

//    qreal c = 0;

//    for (qint32 i = 0; i < nl.size(); i++) {

//        QStringList in = nl.at(i).toElement().attribute("d").split(" ");

//        if (in.contains("s", Qt::CaseInsensitive)) continue;
//        if (in.contains("c", Qt::CaseInsensitive)) continue;

//        QLineF line;
//        line.setP1(QPointF(0, 0));

//        QLineF next;
//        next.setP1(QPointF(0, 0));

//        if (start == 2) {
//            line.setP2(QPointF(in.at(start).split(",").first().toDouble(), in.at(start).split(",").last().toDouble()));
//            msgLen = digitStream(line.length()).toInt();
//        }

//        // Deprecated:
//        // int index = 1;
//        // QLineF mainLine;
//        // setMainLine(mainLine, in, index);
//        // msgLen = secretFromLine(2, mainLine, in).toInt();

//        for (int i = start + 1; i < in.size() - 1; i++) {

//            line.setP2(QPointF(in.at(i).split(",").first().toDouble(), in.at(i).split(",").last().toDouble()));
//            next.setP2(QPointF(in.at(i + 1).split(",").first().toDouble(), in.at(i + 1).split(",").last().toDouble()));

//            if (linesNotParallel(line, next)) {
//                c += QString::number(line.length(), 'g', 8).toDouble();
//                continue;
//            }

//            msgBytesAsString.append(digitStream(line.length() + c));
//            c = 0;

//            if (msgBytesAsString.size() / 3 >= msgLen) break;

//            // Deprecated:
//            // if (i == index) {
//            //     if (msgBytesAsString.size() / 3 >= msgLen) break;
//            //     setMainLine(mainLine, in, index);
//            //     continue;
//            // }
//            // msgBytesAsString += secretFromLine(i, mainLine, in);
//        }

//        if (msgBytesAsString.size() / 3 >= msgLen) break;

//        start--;

//        c += QString::number(next.length(), 'g', 8).toDouble();
//    }

    msgBytesAsString.truncate(msgLen * 3);

    //FIXME1
    if (mIsDebug) debugMessage(QString(NUM_MSG_LEN_NUMS - QString::number(msgLen).size(), '0') + QString::number(msgLen) + QString::number(settings) + msgBytesAsString);

    QByteArray msgBytes;
    int size = msgBytesAsString.size() / 3;
    for (int i = 0; i < size; i++) {
        msgBytes.append(QChar(msgBytesAsString.left(3).toInt()));
        msgBytesAsString = msgBytesAsString.mid(3);
    }

    emit sendMessage(msgBytes, isCompress, isEncrypt);

    //FIXME1
    if (mIsDebug) qDebug() << msgBytes;

    return true;
}

void Vector::save(QString &name) {
    QFile f(name);
    if (!f.open(QFile::WriteOnly)) {
        emit writeToConsole("[Vector] Cannot open file:" + name + " to save the modified svg file.\n");
        return;
    }

    QTextStream stream(&f);
    stream << mXml;

    f.close();
}

bool Vector::linesNotParallel(QLineF & line, QLineF & nextLine) {
    qreal angle = line.angleTo(nextLine);
    if (angle > ANGLE_PREC) angle = 360 - angle;
    return angle > ANGLE_PREC;
}

qreal Vector::polyLineLength(QStringList & in) {

    qreal c = 0;
    bool m = in.at(0) == "m";
    QLineF line;

    line.setP1(QPointF(0, 0));
    if (!m) line.setP1(QPointF(in.at(1).split(",").first().toDouble(), in.at(1).split(",").last().toDouble()));

    for (int i = 2; i < in.size(); i++) {

        line.setP2(QPointF(in.at(i).split(",").first().toDouble(), in.at(i).split(",").last().toDouble()));

        c = c + line.length();

        if (!m) line.setP1(line.p2());
    }

    return c;
}

void Vector::preparePolyLine(QStringList & in) {

    bool m = in.at(0) == "m";

    QLineF line;
    QLineF nextLine;

    line.setP1(QPointF(0, 0));
    nextLine.setP1(QPointF(0, 0));
    if (!m) {
        line.setP1(QPointF(in.at(1).split(",").first().toDouble(), in.at(1).split(",").last().toDouble()));
        nextLine.setP1(QPointF(in.at(2).split(",").first().toDouble(), in.at(2).split(",").last().toDouble()));
    }

    for (int i = 2; i < in.size() - 1; i++) {

        line.setP2(QPointF(in.at(i).split(",").first().toDouble(), in.at(i).split(",").last().toDouble()));
        nextLine.setP2(QPointF(in.at(i + 1).split(",").first().toDouble(), in.at(i + 1).split(",").last().toDouble()));

        if (!linesNotParallel(line, nextLine)) {
            in.removeAt(i);
            if (m) {
                nextLine.setP2(nextLine.p2() + line.p2());
                in[i] = QString::number(nextLine.p2().x(), 'g', 8) + ',' + QString::number(nextLine.p2().y(), 'g', 8);
            }
            i--;
        }

        if (!m) {
            line.setP1(line.p2());
            nextLine.setP1(nextLine.p2());
        }
    }
}

QString Vector::addToReal(qreal real, int inc, int fppos) {

    int sign = qFabs(real) / real;

    QString digits = digitStream(qFabs(real), fppos);

    QString tmp = QString::number(digits.toInt() + inc);

    tmp.prepend(QString(8 - tmp.size(), '0'));

    qreal numb = streamToReal(tmp, fppos) * sign;

    return QString::number(numb, 'g', 8);
}

QString Vector::setEven(QString number, bool even, int & direction) {

    int rightmost;

    rightmost = digitAt(number);

    int increment = ((rightmost % 2) ^ !even);

    if (direction > 0) {
        if ((rightmost - increment) < 0) direction++;
        else {
            increment = -increment;
            direction--;
        }
    } else {
        if ((rightmost + increment) / 10) {
            increment = -increment;
            direction--;
        } else direction++;
    }

    setDigitAt(number, rightmost += increment);

    return number;
}

//void Vector::iluminatePoints(QByteArray & arr) {
//    QFile f(mImageName);
//    f.open(QIODevice::ReadOnly);

//    QDomDocument d("svgFile");
//    d.setContent(&f);
//    f.close();

//    QDomNodeList l = d.elementsByTagName("path");

//    QDomElement elem;

//    QDomElement gelem;

//    for (int i = 0; i < l.size(); i++) {
//        QStringList in = l.at(i).toElement().attribute("d").split(" ");
//        if (in.contains("s", Qt::CaseInsensitive)) continue;
//        if (in.contains("c", Qt::CaseInsensitive)) continue;

//        bool m = in.at(0) == "m";
//        QPointF point(in.at(1).split(",").first().toDouble(), in.at(1).split(",").last().toDouble());

//        elem = d.createElement("rect");
//        elem.setAttribute("style", "fill:#ff0000;stroke:#ff0000;stroke-opacity:1");
//        elem.setAttribute("id", "rect0");
//        elem.setAttribute("width", QString::number(SHOW_POINTS_W_AND_H, 'g', 1));
//        elem.setAttribute("height", QString::number(SHOW_POINTS_W_AND_H, 'g', 1));
//        elem.setAttribute("x", QString::number(point.x() - SHOW_POINTS_W_AND_H / 2, 'g', 8));
//        elem.setAttribute("y", QString::number(point.y() - SHOW_POINTS_W_AND_H / 2, 'g', 8));

//        gelem = l.at(i).parentNode().toElement();
//        gelem.appendChild(elem);

//        for (int j = 2; j < in.size(); j++) {
//            if (m) {
//                point += QPointF(in.at(j).split(",").first().toDouble(), in.at(j).split(",").last().toDouble());
//            }

//            elem = d.createElement("rect");
//            elem.setAttribute("style", "fill:#ff0000;stroke:#ff0000;stroke-opacity:1");
//            elem.setAttribute("id", "rect0");
//            elem.setAttribute("width", QString::number(SHOW_POINTS_W_AND_H, 'g', 1));
//            elem.setAttribute("height", QString::number(SHOW_POINTS_W_AND_H, 'g', 1));
//            elem.setAttribute("x", QString::number(point.x() - SHOW_POINTS_W_AND_H / 2, 'g', 8));
//            elem.setAttribute("y", QString::number(point.y() - SHOW_POINTS_W_AND_H / 2, 'g', 8));

//            gelem.appendChild(elem);
//        }
//    }

//    arr = d.toByteArray();
//}

QString Vector::setDigitAt(QString number, int digit, int pos) {
    QString mantis = number.split(QRegExp("[eE]")).first();

    mantis.append(QString(9 - mantis.size(), '0'));

    pos = mantis.size() - 1 - pos;

    number.replace(pos, 1, QString::number(digit).at(0));

    return number;
}

int Vector::digitAt(QString number, int pos) {
    QString mantis = number.split(QRegExp("[eE]")).first();

    mantis.append(QString(9 - mantis.size(), '0'));

    QChar digit = mantis.at(mantis.size() - 1 - pos);

    return digit.digitValue();
}

QString Vector::setEven(QString number, bool even) {

    int rightmost = digitAt(number);

    int increment = ((rightmost % 2) ^ !even);
    if (!even) increment = -increment;

    return setDigitAt(number, rightmost += increment);
}

bool Vector::isEven(QString number) {
    return !(digitAt(number) % 2);
}

void Vector::setMainLine(QLineF & line, QStringList & in,int & index) {
    if (index == 1)
        line.setP1(QPointF(in.at(index).split(",").first().toDouble(), in.at(index).split(",").last().toDouble()));
    else
        line.setP1(line.p2());

    line.setP2(line.p1());
    index++;

    for (; index < in.size(); index++) {
        line.setP2(line.p2() + QPointF(in.at(index).split(",").first().toDouble(), in.at(index).split(",").last().toDouble()));
        if (!isEven(in.at(index).split(",").last())) break;
    }

    index++;
    line.setP2(line.p2() + QPointF(in.at(index).split(",").first().toDouble(), in.at(index).split(",").last().toDouble()));
}

QString Vector::secretFromLine(int lineNumber, QLineF & mainLine, QStringList & in) {
    return Stegosum::digitStream((in.at(lineNumber).split(",").first().toDouble() * mainLine.length()) / mainLine.dx(), mFPPos);
}

void Vector::Number::normalize() {

    QStringList e = this->mFloatingPoint.last().split(QRegExp("[eE]"));

    if (e.size() > 1) {
        int exponent = e.last().toInt();

        QString beforeDot = this->mFloatingPoint.first();
        QString afterDot = e.first();

        QString whole = beforeDot + afterDot;
        int pos = beforeDot.size() + exponent;

        QString tmp(whole);
        tmp.chop(whole.size() - pos);

        beforeDot = tmp;
        beforeDot.append(QString(pos - whole.size(), '0'));

        afterDot = whole.mid(pos);
        afterDot.prepend(QString(-pos, '0'));

        this->mFloatingPoint.first() = beforeDot;
        this->mFloatingPoint.last() = afterDot;
    }
}

void Vector::Number::fillzero(Number & other) {
    int len = this->mFloatingPoint.first().size() - other.mFloatingPoint.first().size();

    if (len > 0) {
        other.mFloatingPoint.first().prepend(QString(len, '0'));
    } else if (len < 0) {
        this->mFloatingPoint.first().prepend(QString(-len, '0'));
    }

    len = this->mFloatingPoint.last().size() - other.mFloatingPoint.last().size();

    if (len > 0) {
        other.mFloatingPoint.last().append(QString(len, '0'));
    } else if (len < 0) {
        this->mFloatingPoint.last().append(QString(-len, '0'));
    }
}

Vector::Number Vector::Number::operator- (const Number & other) {

    Number mensenec(mFloatingPoint);
    Number mensitel(other.mFloatingPoint);
    Number result("");

    if (mensenec < mensitel) {
        mensenec.swap(mensitel);
    }

    mensenec.fillzero(mensitel);

    QString r;
    bool c = false;
    for (int i = mensenec.size() - 1; i >= 0; i--) {
        int up = mensenec.at(i).digitValue();
        int down = mensitel.at(i).digitValue() + c;
        if ((c = down > up)) up += 10;
        r.prepend(QString::number(up - down));
    }

    result.mFloatingPoint.first() = r.left(mensenec.mFloatingPoint.first().size());
    result.mFloatingPoint.last() = r.right(mensenec.mFloatingPoint.last().size());

    result.clean();

    return result;
}

void Vector::Number::clean() {
    int i;
    for (i = 0; i < mFloatingPoint.first().size(); i++) {
        if (mFloatingPoint.first().at(i) != '0') break;
    }
    mFloatingPoint.first() = mFloatingPoint.first().mid(i);

    int c = mFloatingPoint.last().size();
    for (i = 0; i < mFloatingPoint.last().size(); i++) {
        if (mFloatingPoint.last().at(--c) != '0') break;
    }
    mFloatingPoint.last().chop(i);
}

void Vector::Number::swap(Number & other) {
    QStringList tmp(this->mFloatingPoint);
    this->mFloatingPoint = other.mFloatingPoint;
    other.mFloatingPoint = tmp;
}

bool Vector::Number::operator> (const Number & other) {
    Number left(mFloatingPoint);
    Number right(other.mFloatingPoint);
    left.fillzero(right);
    return left.mFloatingPoint.join("") > right.mFloatingPoint.join("");
}

bool Vector::Number::operator< (const Number & other) {
    Number left(mFloatingPoint);
    Number right(other.mFloatingPoint);
    left.fillzero(right);
    return left.mFloatingPoint.join("") < right.mFloatingPoint.join("");
}

bool Vector::Number::operator>= (const Number & other) {
    Number left(mFloatingPoint);
    Number right(other.mFloatingPoint);
    left.fillzero(right);
    return left.mFloatingPoint.join("") >= right.mFloatingPoint.join("");
}

bool Vector::Number::operator<= (const Number & other) {
    Number left(mFloatingPoint);
    Number right(other.mFloatingPoint);
    left.fillzero(right);
    return left.mFloatingPoint.join("") <= right.mFloatingPoint.join("");
}

bool Vector::Number::operator== (const Number & other) {
    Number left(mFloatingPoint);
    Number right(other.mFloatingPoint);
    left.fillzero(right);
    return left.mFloatingPoint.join("") == right.mFloatingPoint.join("");
}

Vector::Number & Vector::Number::operator= (const Number & other) {
    mFloatingPoint = other.mFloatingPoint;
    return *this;
}

