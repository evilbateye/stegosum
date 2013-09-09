#include <algorithm>
#include <cstdarg>
#include "stegosum.h"
#include "mainwindow.h"

Stegosum::Stegosum(QWidget *parent) :
    QThread(parent), mEncode(true), mIsEncrypt(false)
{
}

void Stegosum::run()
{
    if (mEncode) emit succes(Encode());
    else emit succes(Decode());
}

void Stegosum::setUp(MainWindow * mw, bool encode, bool isCompress, bool isEncrypt, bool isLookAhead, bool isMeta, int fppos, bool isfpposmax)
{
    mMsg = mw->mSecretBytes;
    mImageName = mw->mFileName;
    mColors = mw->mColors;
    mEncode = encode;
    mPassword = mw->mPassword;
    mKey = qChecksum(mPassword.toStdString().c_str(), mPassword.size());
    mIsCompress = isCompress;
    mIsEncrypt = isEncrypt;
    mIsLookAhead = isLookAhead;
    mIsMeta = isMeta;
    mFPPos = fppos;
    mIsFPPosMax = isfpposmax;
}

qreal Stegosum::streamToReal(QString digitStream, int ffpos) {
    ffpos -= 7;
    digitStream.prepend(QString(-ffpos + 1, '0'));
    digitStream.insert(ffpos > 0 ? ffpos : 1, '.');
    return digitStream.toDouble();
}

QString Stegosum::digitStream(qreal number, int fppos) {
    fppos -= 7;
    QString numberStr = QString::number(number, 'f', 8 - fppos);
    numberStr.prepend(QString(fppos - numberStr.split(".").first().size(), '0'));
    numberStr.remove(QRegExp("\\."));
    numberStr.remove(0, -(fppos - 1));
    return numberStr;
}
