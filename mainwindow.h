#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDebug>
#include "secretmsgtextedit.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QProgressBar>
#include "pointgenthread.h"
#include "clickablelabel.hpp"
#include <QtCrypto/QtCrypto>
#include <QBuffer>
#include <QFile>
#include <QScrollArea>
#include <QScrollBar>
#include <QBitmap>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void on_actionEncode_triggered();
    void on_actionDecode_triggered();
    void on_lineEdit_textChanged(const QString &arg1);

    void secretMsgTextEditPressHandler();
    void secretMsgTextChangedHandler();
    void compressSliderChangedHandler(int change);
    void threadEncodeHandler(bool succ);
    void setProgress(int val);
    void receiveSecretMsg(QByteArray msgBytes, bool compressed, bool encrypted);

    void on_encryptCheckBox_clicked();

    void on_radioButtonData_clicked();
    void on_radioButtonText_clicked();
    void on_openDataButton_clicked();

    void slotZoom(int scrollDelta);
    void slotZoomIn();
    void slotZoomOut();
    void slotNormalSize();
    void slotOnlyLSBRed();
    void slotOnlyLSBGreen();
    void slotOnlyLSBBlue();
    void slotOnlyLSBAll();
    void slotOnlyLSBNormal();

    void slotChangeStegoImgVisib(bool);

    void slotImgMoveTool(QPoint);

    void on_makeAnalysisButton_clicked();

    void on_red_radio_clicked(bool checked);

    void on_green_radio_clicked(bool checked);

    void on_blue_radio_clicked(bool checked);

    void on_lookAheadRadio_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    QString mPassword;
    QString mFileName;
    QString mDataFileName;
    QImage mImg;
    PointGenThread mPointGenThread;
    QProgressBar * mPgBar;
    QByteArray mSecretBytes;
    QByteArray mUnchangedSecretBytes;
    int mNumWritableChars;
    QCA::Initializer mInit;
    QCA::Cipher mCipher;

    float mScaleFactor;
    QImage mModifiedImg;
    QImage mModifiedStego;
    PointGenThread::Color mLastModified;

    QString mAnalysisOutFileName;

    quint8 mNumOfSelectedColors;

    void updateStatusBar();
    void setNumMax();
    QByteArray encrypt(QByteArray msg);
    QByteArray decrypt(QByteArray msg);
    void openImage(QString name);

    void scaleImage(float factor);

    bool convertToLSB(QImage & image, PointGenThread::Color color, ClickableLabel * label = 0);
    bool resetImages(PointGenThread::Color color);
    void adjustMyScrollBars();
    void adjustMySize(QImage & image, ClickableLabel *label);
};

#endif // MAINWINDOW_H

