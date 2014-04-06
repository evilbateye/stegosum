#ifndef STEGOSUM_H
#define STEGOSUM_H

#include <QThread>
#include <QWidget>
#include <QFileDialog>
#include <QDebug>
#include <QProgressBar>
#include <QVector>
#include <QBuffer>
#include <QMainWindow>

#include "utils.hpp"

class MainWindow;

class Stegosum : public QThread
{
    Q_OBJECT
public:
    explicit Stegosum(QWidget *parent = 0);
    void run();
    virtual bool Encode() = 0;
    virtual bool Decode() = 0;
    void setUp(MainWindow * mw, bool encode = true, bool isCompress = false, bool isEncrypt = false, bool isLookAhead = false, int fppos = 8, bool isfpposmax = false);
    //virtual QImage convertToLSB() = 0;
    virtual void saveStegoImg(QString & name) = 0;

    virtual QPair<QImage, QImage> scaleImgs(float factor) = 0;
    virtual void setSelectedImgs(Utils::DisplayImageColor color) = 0;
    virtual QPair<QImage, QImage> getImgs(Utils::DisplayImageColor color = Utils::COLOR_NONE) = 0;

    bool isRaster() { return mIsRaster; }

    inline bool isEncode() { return mEncode; }    

signals:
    void succes(bool succ);
    void updateProgress(int value);
    void setMaximum(int value);
    void sendMessage(QByteArray message, bool compressed, bool encrypted);
    void writeToConsole(QString string);
    void writeToStatus(QString string);

public slots:

protected:
    QByteArray mMsg;
    quint16 mKey;
    QString mPassword;
    Utils::EncodeColorsObj mColors;
    bool mEncode;
    bool mIsCompress;
    bool mIsEncrypt;
    bool mIsLookAhead;
    int mFPPos;
    bool mIsFPPosMax;
    bool mIsDebug;
    bool mIsRaster;
    Utils::DisplayImageColor mSelColor;
};

#endif // POINTGENTHREAD_H
