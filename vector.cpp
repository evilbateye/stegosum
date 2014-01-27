#include <QLineF>

#include "vector.h"
#include <QtSvg>
#include "utils.hpp"

#define NUM_MSG_LEN_NUMS 8
#define ANGLE_PREC 1
//#define FLOATING_POINT_POS 9
#define SHOW_POINTS_W_AND_H 5

#include <iostream>

void debugMessage(QString msg) {

    while (!msg.isEmpty()) {

        std::cout << msg.left(8).toStdString() << " ";

        msg.remove(0, 8);
    }

    std::cout << std::endl;
}

Vector::Vector(const QString &name)
{
    QFile file(name);

    file.open(QIODevice::ReadOnly);

    mSelXmlIn[Utils::COLOR_NONE] = file.readAll();

    file.close();

    QBuffer buff(&mSelXmlIn[Utils::COLOR_NONE]);
    QImage i;
    i.load(&buff, "svg");

    mSize = i.size();

    mSelColor = Utils::COLOR_NONE;
    mIsRaster = false;
}

Vector::~Vector() {
}

void Vector::setSelected(Utils::Color color) {

    if (color == Utils::COLOR_PREV) color = mSelColor;

    mSelColor = color;

    if (color == Utils::COLOR_NONE) return;

    if (mSelXmlIn[color].isEmpty()) {
        mSelXmlIn[color] = mSelXmlIn[Utils::COLOR_NONE];
        iluminatePoints(mSelXmlIn[color]);
    }

    if (mSelXmlOut[color].isEmpty() && !mSelXmlOut[Utils::COLOR_NONE].isEmpty()) {
        mSelXmlOut[color] = mSelXmlOut[Utils::COLOR_NONE];
        iluminatePoints(mSelXmlOut[color]);
    }
}

QPair<QImage, QImage> Vector::get(Utils::Color color) {

    QBuffer buff;
    QImage in, out;

    buff.setBuffer(&mSelXmlIn[color]);
    in.load(&buff, "svg");
    buff.close();

    buff.setBuffer(&mSelXmlOut[color]);
    out.load(&buff, "svg");
    buff.close();

    return qMakePair(in, out);
}

QPair<QImage, QImage> Vector::scale(float factor) {
    QSvgRenderer r(mSelXmlIn[mSelColor]);
    QPainter p;
    QImage in(mSize * factor, QImage::Format_ARGB32);

    in.fill(0xffffffff);
    p.begin(&in);
    r.render(&p);
    p.end();

    if (!mSelXmlOut[mSelColor].isEmpty()){

        QImage out(mSize * factor, QImage::Format_ARGB32);
        r.load(mSelXmlOut[mSelColor]);
        out.fill(0xffffffff);
        p.begin(&out);
        r.render(&p);
        p.end();

        return qMakePair(in, out);
    }

    return qMakePair(in, QImage());
}

qreal Vector::totalMessageLength(QString msg, int fpppos) {

    qreal length = 0;

    while (!msg.isEmpty()) {

        length += Stegosum::streamToReal(msg.left(8), fpppos);

        msg.remove(0, 8);
    }

    return length;
}

bool Vector::isLineSupported(QStringList & in) {
    if (in.contains("q", Qt::CaseInsensitive)) return false;
    if (in.contains("c", Qt::CaseInsensitive)) return false;
    if (in.contains("s", Qt::CaseInsensitive)) return false;

    if (in.contains("l", Qt::CaseInsensitive)) return false;
    if (in.contains("h", Qt::CaseInsensitive)) return false;
    if (in.contains("v", Qt::CaseInsensitive)) return false;

    if (in.contains("z", Qt::CaseInsensitive)) return false;

    return true;
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

        if (!isLineSupported(in)) continue;

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

int Vector::computeDifference(qreal precise, qreal a, qreal b)
{
    qreal rounded = QLineF(0, 0, a, b).length();

    return Stegosum::digitStream(precise, mFPPos).toInt() - Stegosum::digitStream(rounded, mFPPos).toInt();
}

bool Vector::precisionCorrection(qreal precise, QString & A, QString & B)
{
    qreal realA = A.toDouble();
    qreal realB = B.toDouble();

    int dif = computeDifference(precise, realA, realB);

    if (!dif) return true;

    qreal & tinker = realA < realB ? realA : realB;

    while (dif > 0) {

        tinker = addToReal(tinker, 1, mFPPos).toDouble();

        dif = computeDifference(precise, realA, realB);
    }

    while (dif < 0) {

        tinker = addToReal(tinker, -1, mFPPos).toDouble();

        dif = computeDifference(precise, realA, realB);
    }

    A = QString::number(realA, 'g', 8);
    B = QString::number(realB, 'g', 8);

    return dif == 0;
}

bool Vector::Encode() {

    //FIXME1
    if (mIsDebug) {
        QFile file;
        file.setFileName("/home/evilbateye/develop/CD/stegosum-build-desktop-Qt_4_8_3_in_PATH__System__Release/drawing.svg");
        file.open(QFile::ReadOnly);
        mSelXmlIn[Utils::COLOR_NONE] = file.readAll();
        file.close();
        mMsg = "aaaaaaaaaaaaaaaaaaaa";
        mPassword = "";
    }

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


    if (mSelXmlIn[Utils::COLOR_NONE].isEmpty()) {
        emit writeToConsole("[Vector] Cannot open input vector image for encoding.\n");
        return false;
    }

    if (!doc.setContent(mSelXmlIn[Utils::COLOR_NONE])) {
        emit writeToConsole("[Vector] Cannot parse input vector image for encoding.\n");
        return false;
    }

    QDomNodeList nl = doc.elementsByTagName("path");

    QString msgBytesAsString;

    bool firstTime = true;

    for (qint32 i = 0; i < nl.size(); i++) {

        QStringList in = nl.at(i).toElement().attribute("d").split(" ");

        if (!isLineSupported(in)) continue;

        // First polyline's first point adjustment.
        if (firstTime) {
            int random;

            // Initialize pseudorandom number generator
            // using user's password and first point's Y coord.
            qsrand(mKey ^ digitStream(in.at(1).split(",").last().toDouble(), 9).toInt());

            // Randomize secret message length for safety
            // (length info isn't encrypted).
            int msgLen;
            random = qrand() % 100000000;
            msgLen = random + mMsg.size();
            if (msgLen >= 100000000) msgLen = msgLen - 100000000;

            if (mIsDebug) std::cout << "[E] mMsg.size() = " << mMsg.size() << "; randomized = " << msgLen << std::endl;

            // And insert as new length for encoding.
            msgBytesAsString.append(QString(NUM_MSG_LEN_NUMS - QString::number(msgLen).size(), '0') + QString::number(msgLen));

            // Same with settings.
            int settings = 0;
            settings = mIsCompress;
            settings = (mIsEncrypt << 1) | settings;

            if (mIsDebug) std::cout << "[E] settings = " << settings << "; ";

            random = qrand() % 10;
            settings = random + settings;
            if (settings >= 10) settings = settings - 10;

            if (mIsDebug) std::cout << "randomized = " << settings << std::endl;

            msgBytesAsString.append(QString::number(settings));

            for (int i = 0; i < mMsg.size(); i++) {
                quint8 byte = mMsg[i];
                QString sbyte = QString::number(byte);
                QString append = QString(3 - sbyte.size(), '0') + sbyte;
                msgBytesAsString.append(append);
            }

            //FIXME1
            if (mIsDebug) {
                std::cout << "[E] ";
                debugMessage(msgBytesAsString);
            }

            // Auto compute the floating point position of the message
            // based on the length of all encodable lines.
            if (mIsFPPosMax) {

                // Get the total sum of all lines lengths.
                qreal totalLength = 0;
                for (qint32 i = 0; i < nl.size(); i++) {
                    QStringList in = nl.at(i).toElement().attribute("d").split(" ");

                    if (!isLineSupported(in)) continue;

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

            // Randomize floating point position info.
            random = qrand() % 10;
            int fppos = random + mFPPos;
            if (fppos >= 10) fppos = fppos - 10;

            //FIXME1
            if (mIsDebug) std::cout << "[E] mFPPos = " << mFPPos << "; randomized = " << fppos << "; random number = " << random << std::endl << std::endl;

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

                // streamToReal function expects the input
                // to be a string of length 8
                QString tmpstr = msgBytesAsString.left(8);
                if (tmpstr.size() < 8) tmpstr.append(QString(8 - tmpstr.size(), '0'));

                c = Stegosum::streamToReal(tmpstr, mFPPos);

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

                    break;
                }

                msgBytesAsString = msgBytesAsString.mid(8);

                a = (c * line.dx()) / QString::number(line.length(), 'g', 8).toDouble();
                b = (c * line.dy()) / QString::number(line.length(), 'g', 8).toDouble();

                QString A = QString::number(a, 'g', 8);
                QString B = QString::number(b, 'g', 8);

                if (!precisionCorrection(c, A, B)) {
                    emit writeToConsole("[Vector] Precision correction failed.\n");
                    return false;
                }

                out.insert(j + encoded++, A + "," + B);

                asum = QString::number(asum + A.toDouble(), 'g', 8).toDouble();
                bsum = QString::number(bsum + B.toDouble(), 'g', 8).toDouble();

                if (msgBytesAsString.isEmpty()) {

                    // Msg has been successfuly encoded,
                    // but we still need to adjust this lines end point's coords.
                    a = QString::number(line.dx() - asum, 'g', 8).toDouble();
                    b = QString::number(line.dy() - bsum, 'g', 8).toDouble();
                    out[j + encoded] = QString::number(a, 'g', 8) + "," + QString::number(b, 'g', 8);
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

    mSelXmlOut[Utils::COLOR_NONE] = doc.toByteArray();
    mSelXmlOut[Utils::COLOR_ILUM].clear();

    //FIXME1
    if (mIsDebug){
        QFile of("/home/evilbateye/develop/CD/stegosum-build-desktop-Qt_4_8_3_in_PATH__System__Release/drawing_out.svg");
        of.open(QFile::WriteOnly);
        QTextStream ts(&of);
        ts << mSelXmlOut[Utils::COLOR_NONE];
        of.close();
        Decode();
    }

    return true;
}

bool Vector::Decode() {

    QDomDocument doc("svgFile");

    //FIXME1
    if (mIsDebug) {
        QFile file;
        file.setFileName("/home/evilbateye/develop/CD/stegosum-build-desktop-Qt_4_8_3_in_PATH__System__Release/drawing_out.svg");
        file.open(QFile::ReadOnly);
        mSelXmlIn[Utils::COLOR_NONE] = file.readAll();
        file.close();
    }

    if (mSelXmlIn[Utils::COLOR_NONE].isEmpty()) {
        emit writeToConsole("[Vector] Cannot open input vector file for decoding.\n");
        return false;
    }

    if (!doc.setContent(mSelXmlIn[Utils::COLOR_NONE])) {
        emit writeToConsole("[Vector] Cannot parse input vector file for decoding.\n");
        return false;
    }

    QDomNodeList nl = doc.elementsByTagName("path");

    int fppos;
    int randomMsgLen;
    int randomSettings;
    for (qint32 i = 0; i < nl.size(); i++) {

        QStringList in = nl.at(i).toElement().attribute("d").split(" ");

        if (!isLineSupported(in)) continue;

        qsrand(mKey ^ digitStream(in.at(1).split(",").last().toDouble(), 9).toInt());

        randomMsgLen = qrand() % 100000000;
        randomSettings = qrand() % 10;
        int randomFppos = qrand() % 10;

        fppos = digitAt(in.at(1).split(",").first());

        if (mIsDebug) std::cout << "[D] ramdomized = " << fppos;

        if (fppos < randomFppos) fppos += 10;
        fppos -= randomFppos;

        if (mIsDebug) std::cout << "; fppos = " << fppos << "; random number = " << randomFppos << std::endl;

        break;
    }

    QString msgBytesAsString;
    int firstPolyL = 0;
    int firstL = 2;
    qreal prevDistance = 0.0;

    nextPointSecret(nl, msgBytesAsString, firstPolyL, firstL, prevDistance, fppos);
    int msgLen = msgBytesAsString.left(8).toInt();

    if (mIsDebug) std::cout << "[D] randomized = " << msgLen << "; ";

    if (msgLen < randomMsgLen) msgLen += 100000000;
    msgLen -= randomMsgLen;

    if (mIsDebug) std::cout << "msgLen = " << msgLen << std::endl;

    if (!msgLen) {
        emit writeToConsole("[Vector] Decoded length of the secret message is 0. Wrong password and/or the image doesn't containg any secret message.\n");
        return false;
    }

    msgBytesAsString.clear();

    nextPointSecret(nl, msgBytesAsString, firstPolyL, firstL, prevDistance, fppos);
    int settings = msgBytesAsString.at(0).digitValue();

    if (mIsDebug) std::cout << "[D] randomized = " << settings << "; ";

    if (settings < randomSettings) settings += 10;
    settings -= randomSettings;

    if (mIsDebug) std::cout << "settings = " << settings << std::endl;

    bool isCompress = settings & 1;
    bool isEncrypt = (settings >> 1) & 1;
    msgBytesAsString.remove(0, 1);

    while (msgBytesAsString.size() / 3 < msgLen) {

        if (nextPointSecret(nl, msgBytesAsString, firstPolyL, firstL, prevDistance, fppos)) {
            emit writeToConsole("[Vector] Decoding failed. Decoded length of the secret message is too big. Image doesn't have a secret message, maybe?\n");
            return false;
        }
    }

    msgBytesAsString.truncate(msgLen * 3);

    //FIXME
    if (mIsDebug) {
        std::cout << "[D] ";
        debugMessage(QString(NUM_MSG_LEN_NUMS - QString::number(msgLen).size(), '0') + QString::number(msgLen) + QString::number(settings) + msgBytesAsString);
    }

    QByteArray msgBytes;
    int size = msgBytesAsString.size() / 3;
    for (int i = 0; i < size; i++) {
        msgBytes.append(QChar(msgBytesAsString.left(3).toInt()));
        msgBytesAsString = msgBytesAsString.mid(3);
    }

    emit sendMessage(msgBytes, isCompress, isEncrypt);

    return true;
}

void Vector::save(QString &name) {

    QFile f(name);

    if (!f.open(QFile::WriteOnly)) {
        emit writeToConsole("[Vector] Cannot open file:" + name + " to save the modified svg file.\n");
        return;
    }

    QTextStream stream(&f);

    stream << mSelXmlOut[Utils::COLOR_NONE];

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

void Vector::iluminatePoints(QByteArray & arr) {

    QDomDocument d("svgFile");
    d.setContent(arr);

    QDomNodeList l = d.elementsByTagName("path");

    QDomElement elem;

    QDomElement gelem;

    for (int i = 0; i < l.size(); i++) {
        QStringList in = l.at(i).toElement().attribute("d").split(" ");

        if (!isLineSupported(in)) continue;

        bool m = in.at(0) == "m";
        QPointF point(in.at(1).split(",").first().toDouble(), in.at(1).split(",").last().toDouble());

        elem = d.createElement("rect");
        elem.setAttribute("style", "fill:#ff0000;stroke:#B3B3B3;stroke-opacity:1;stroke-width:1");
        elem.setAttribute("id", "rect0");
        elem.setAttribute("width", QString::number(SHOW_POINTS_W_AND_H, 'g', 1));
        elem.setAttribute("height", QString::number(SHOW_POINTS_W_AND_H, 'g', 1));
        elem.setAttribute("x", QString::number(point.x() - SHOW_POINTS_W_AND_H / 2, 'g', 8));
        elem.setAttribute("y", QString::number(point.y() - SHOW_POINTS_W_AND_H / 2, 'g', 8));

        gelem = l.at(i).parentNode().toElement();
        gelem.appendChild(elem);

        for (int j = 2; j < in.size(); j++) {
            if (m) {
                point += QPointF(in.at(j).split(",").first().toDouble(), in.at(j).split(",").last().toDouble());
            } else {
                point = QPointF(in.at(j).split(",").first().toDouble(), in.at(j).split(",").last().toDouble());
            }

            elem = d.createElement("rect");
            elem.setAttribute("style", "fill:#ff0000;stroke:#B3B3B3;stroke-opacity:1;stroke-width:1");
            elem.setAttribute("id", "rect0");
            elem.setAttribute("width", QString::number(SHOW_POINTS_W_AND_H, 'g', 1));
            elem.setAttribute("height", QString::number(SHOW_POINTS_W_AND_H, 'g', 1));
            elem.setAttribute("x", QString::number(point.x() - SHOW_POINTS_W_AND_H / 2, 'g', 8));
            elem.setAttribute("y", QString::number(point.y() - SHOW_POINTS_W_AND_H / 2, 'g', 8));

            gelem.appendChild(elem);
        }
    }

    arr = d.toByteArray();
}

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
