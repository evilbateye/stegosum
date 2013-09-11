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
#include "stegosum.h"
#include "clickablelabel.hpp"
#include <QtCrypto/QtCrypto>
#include <QBuffer>
#include <QFile>
#include <QScrollArea>
#include <QScrollBar>
#include <QBitmap>
#include "utils.hpp"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    QByteArray encrypt(QByteArray msg);
    QByteArray decrypt(QByteArray msg);
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
    void slotOnlyLSBRed(bool checked);
    void slotOnlyLSBGreen(bool checked);
    void slotOnlyLSBBlue(bool checked);
    void slotOnlyLSBAll(bool checked);
    void slotOnlyLSBNormal(bool checked);
    void slotViewPoints(bool checked);

    void slotChangeStegoImgVisib(bool);

    void slotImgMoveTool(QPoint);

    void on_makeAnalysisButton_clicked();

    void on_red_radio_clicked(bool checked);

    void on_green_radio_clicked(bool checked);

    void on_blue_radio_clicked(bool checked);

    void on_lookAheadRadio_clicked(bool checked);

    void slotWriteToConsole(QString string);

    void on_FPPosSlider_sliderMoved(int position);

    void on_checkBoxMaxFPPosition_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    QString mPassword;
    QString mFileName;
    QString mDataFileName;
    Stegosum * mStegosum;
    QProgressBar * mPgBar;
    QByteArray mSecretBytes;
    QByteArray mUnchangedSecretBytes;
    int mNumWritableChars;
    QCA::Initializer mInit;
    QCA::Cipher mCipher;

    float mScaleFactor;
    QImage mImg;

    QString mAnalysisOutFileName;

    Utils::colorsObj mColors;

    QMenu * mMenuImage;
    QMenu * mMenuView;

    QAction * actionZoom_In_25;
    QAction * actionZoom_Out_25;
    QAction * action_Normal_Size;

    QList<QAction *> mActions;

    bool mIsDebug;

    void updateStatusBar();
    void setNumMax();
    void openImage(QString name);

    void scaleImage(float factor);
    void adjustMyScrollBars();

    friend class Stegosum;
};

#endif // MAINWINDOW_H

