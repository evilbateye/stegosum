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

//! Trieda MainWindow reprezentuje hlavné okno aplikácie.
/*!
Trieda MainWindow, ktorá reprezentuje hlavné okno aplikácie, obsahuje najviac
jednu inštanciu tried Stegosum, Analysis a EncodeColorsObj a využíva pomocné
funkcie z triedy Utils. MainWindow ďalej obsahuje potrebné členské premenné pre
uchovanie tajnej správy, hesla pre šifrovanie a randomizáciu tajnej správy, informácie
pre prácu s krycím obrázkom, informácie pre prácu s tajnou správou vo forme
súboru, metadáta pre kódovanie a dekódovanie tajnej správy (kapacita obrázka, zvolené
farby kódovania) a zobrazovacie prostriedky potrebné pre vizuálnu steganalýzu
(vizuálna steganalýza je prepojená s gui prvkami triedy MainWindow).
*/
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

    void slotWriteToStatus(QString string);

    void on_FPPosSlider_sliderMoved(int position);

    void on_checkBoxMaxFPPosition_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    //! Predstavuje heslo, ktoré vloží používateľ do daného textového poľa.
    QString mPassword;
    //! Predstavuje cestu na disku k načítanému kryciemu obrázku.
    QString mFileName;
    //! Cesta k tajnej správe vo forme súboru na načítanie.
    QString mDataFileName;
    //! Smerník na inštanciu triedy stegosum.
    Stegosum * mStegosum;
    //! Predstavuje dáta tajnej správy vo forme poľa bajtov.
    QByteArray mSecretBytes;
    //! Slúži na odpamätanie nepozmenenej tajnej správy.
    QByteArray mUnchangedSecretBytes;
    //! Označuje kapacitu obrázka ako celkový počet znakov, ktoré je možné zakódovať do obrázka.
    int mNumWritableChars;
    //! Inštancia triedy QCA::Initializer a je nevyhnutné, aby nezanikla po celý čas využívania knižnice QCA.
    QCA::Initializer mInit;
    //! Využíva sa pri vyzuálnej steganalýze, konkrétne pri približovaní alebo vzďaľovaní krycieho obrázka alebo stegoobrázka.
    float mScaleFactor;
    //! Meno výstupného súboru pre steganalýzu.
    QString mAnalysisOutFileName;
    //! Je inštanciou triedy EncodeColorsObject.
    Utils::EncodeColorsObj mColors;

    QMenu * mMenuImage;
    QMenu * mMenuView;
    QProgressBar * mPgBar;

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

