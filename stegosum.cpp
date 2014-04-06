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

void Stegosum::setUp(MainWindow * mw, bool encode, bool isCompress, bool isEncrypt, bool isLookAhead, int fppos, bool isfpposmax)
{
    mMsg = mw->mSecretBytes;

    mColors = mw->mColors;
    mEncode = encode;
    mPassword = mw->mPassword;
    mKey = qChecksum(mPassword.toStdString().c_str(), mPassword.size());
    mIsCompress = isCompress;
    mIsEncrypt = isEncrypt;
    mIsLookAhead = isLookAhead;
    mFPPos = fppos;
    mIsFPPosMax = isfpposmax;

    mIsDebug = mw->mIsDebug;
}
