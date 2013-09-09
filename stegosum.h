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
    void setUp(MainWindow * mw, bool encode = true, bool isCompress = false, bool isEncrypt = false, bool isLookAhead = false, bool isMeta = false, int fppos = 8, bool isfpposmax = false);
    QImage & img() { return mImage; }
    void setImageName(const QString & name) { mImageName = name; }
    virtual void save(QString & name) = 0;
    inline bool isEncode() { return mEncode; }
    static QString digitStream(qreal number, int fppos);
    static qreal streamToReal(QString digitStream, int ffpos);

signals:
    void succes(bool succ);
    void updateProgress(int value);
    void setMaximum(int value);
    void sendMessage(QByteArray message, bool compressed, bool encrypted);
    void writeToConsole(QString string);

public slots:

protected:
    QByteArray mMsg;
    quint16 mKey;
    QString mPassword;
    QImage mImage;
    QString mImageName;
    Utils::colorsObj mColors;
    bool mEncode;
    bool mIsCompress;
    bool mIsEncrypt;
    bool mIsLookAhead;
    bool mIsMeta;
    int mFPPos;
    bool mIsFPPosMax;
};

#endif // POINTGENTHREAD_H
