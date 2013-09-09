//TO DO LIST
/*
  zoom in zoom out scrollbar na okno s obrazkom
  percento zmeny - porovnat obrazky
  steganalyza - ciastocna znalost spravy, porovnat s inymi programamy, aj casovo
  help
  zobrazit zakodovane bity?
  progressbar!
  posli obrazok do threadu ako referenciu, ale pozor nesmie sa menit v hlavnom vlakne
  maximalne sa da dat sprava velkosti 2^21, nemas osetrene
  moznost kodovania aj inych typov suborov
  options
  help
  */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "analysis/samplepairs.h"
#include "analysis/rs.h"
#include "utils.hpp"
#include "raster.h"
#include "vector.h"

#include <QImageReader>
#include <QImageWriter>
#include <QtSvg>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mStegosum(0),
    mNumWritableChars(0),
    mInit(),
    mCipher(QString("aes128"), QCA::Cipher::CBC, QCA::Cipher::DefaultPadding),
    mScaleFactor(1.0),
    mLastModified(Utils::COLOR_NONE)
{
    ui->setupUi(this);

    mPgBar = new QProgressBar(this);
    mPgBar->setVisible(false);
    //ui->statusBar->addPermanentWidget(mPgBar);

    connect(ui->secretMsgView, SIGNAL(textChanged()), this, SLOT(secretMsgTextChangedHandler()));
    connect(ui->compressSlider, SIGNAL(valueChanged(int)), this, SLOT(compressSliderChangedHandler(int)));

    ui->scrollArea->setWidgetResizable(true);
    ui->ImageView->setHSlider(ui->scrollArea->horizontalScrollBar());
    ui->ImageView->setVSlider(ui->scrollArea->verticalScrollBar());
    connect(ui->ImageView, SIGNAL(clicked()), this, SLOT(on_actionOpen_triggered()));
    connect(ui->ImageView, SIGNAL(scrolled(int)), this, SLOT(slotZoom(int)));
    connect(ui->ImageView, SIGNAL(mousePressedAndMoved(QPoint)), this, SLOT(slotImgMoveTool(QPoint)));
    //-----//-----//-----//-----//-----//-----//-----//-----//-----//-----//-----//-----//-----//

    QAction * action;

    mMenuImage = ui->menuBar->addMenu("&Image");

    action = mMenuImage->addAction("&New");
    connect(action, SIGNAL(triggered()), this, SLOT(on_actionOpen_triggered()));

    action = mMenuImage->addAction("Zoom &In (25%)");
    action->setEnabled(false);
    connect(action, SIGNAL(triggered()), this, SLOT(slotZoomIn()));
    actionZoom_In_25 = action;

    action = mMenuImage->addAction("Zoom &Out (25%)");
    action->setEnabled(false);
    connect(action, SIGNAL(triggered()), this, SLOT(slotZoomOut()));
    actionZoom_Out_25 = action;

    action = mMenuImage->addAction("&Normal size");
    action->setEnabled(false);
    connect(action, SIGNAL(triggered()), this, SLOT(slotNormalSize()));
    action_Normal_Size = action;

    mMenuView = ui->menuBar->addMenu("&View");

    action = mMenuView->addAction("&Stego Image");
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(slotChangeStegoImgVisib(bool)));


    ui->scrollArea_2->hide();
    ui->scrollArea_2->setWidgetResizable(true);
    ui->ImageStegoView->setHSlider(ui->scrollArea_2->horizontalScrollBar());
    ui->ImageStegoView->setVSlider(ui->scrollArea_2->verticalScrollBar());
    connect(ui->ImageStegoView, SIGNAL(scrolled(int)), this, SLOT(slotZoom(int)));
    connect(ui->ImageStegoView, SIGNAL(mousePressedAndMoved(QPoint)), this, SLOT(slotImgMoveTool(QPoint)));
    //-----//-----//-----//-----//-----//-----//-----//-----//-----//-----//-----//-----//-----//



//    connect(ui->ImageView, SIGNAL(clicked()), this, SLOT(on_test_slot()));

    //ui->openDataButton->setVisible(false);

    /*testcode*/
    /*mImg.load("/home/evilbateye/Pictures/evilbateye2.png");
    //ui->secretMsgView->setPlainText("01234567");

    mImg.load("/home/evilbateye/Pictures/evilbateye.png");
    ui->secretMsgView->setPlainText("b");

    ui->ImageView->setPixmap(QPixmap::fromImage(mImg));
    ui->encodeButton->setEnabled(true);
    ui->decodeButton->setEnabled(true);

    ui->compressSlider->setEnabled(true);
    ui->encodeMaxCheckBox->setEnabled(true);
    ui->encryptCheckBox->setEnabled(true);

    ui->encodeMaxCheckBox->setChecked(true);
    ui->encryptCheckBox->setChecked(false);
    ui->compressSlider->setValue(0);*/

//    openImage("/home/evilbateye/Pictures/Yoh_Asakura_by_ghettorob21.png");
//    mViewStegoPixmap.convertFromImage(QImage::load("/home/evilbateye/Pictures/Yohs_secret.png"));

    //FIXME1
    //ui->stackedWidget->setCurrentIndex(2);
    //mStegosum = new Vector();
}

bool MainWindow::convertToLSB(QImage & image, Utils::Color color, ClickableLabel * label)
{
    if (image.isNull()) return false;

    if (color == Utils::COLOR_NONE) return false;

    for (int j = 0; j < image.height(); j++) {
        for (int i = 0; i < image.width(); i++) {

            QRgb * pixel = (&reinterpret_cast<QRgb *>(image.scanLine(j))[i]);

            quint8 red = 0x00;
            quint8 green = 0x00;
            quint8 blue = 0x00;

            switch (color) {
                case Utils::COLOR_RED: {
                    red = qRed(* pixel) & 0x01;
                    if (red == 0x01) red = 0xFF;
                    break;
                }

                case Utils::COLOR_GREEN: {
                    green = qGreen(* pixel) & 0x01;
                    if (green == 0x01) green = 0xFF;
                    break;
                }

                case Utils::COLOR_BLUE: {
                    blue = qBlue(* pixel) & 0x01;
                    if (blue == 0x01) blue = 0xFF;
                    break;
                }

                case Utils::COLOR_ALL: {
                    red = qRed(* pixel) & 0x01;
                    if (red == 0x01) red = 0xFF;

                    green = qGreen(* pixel) & 0x01;
                    if (green == 0x01) green = 0xFF;

                    blue = qBlue(* pixel) & 0x01;
                    if (blue == 0x01) blue = 0xFF;
                    break;
                }

                case Utils::COLOR_NONE: break;
                default: break;
            }

            (* pixel) = qRgba(red, green, blue, 0xFF);
        }
    }

    if (!label) return true;

    adjustMySize(image, label);

    return true;
}

void MainWindow::adjustMySize(QImage & image, ClickableLabel * label)
{
    QPixmap tmp;
    tmp.convertFromImage(image.scaled(mScaleFactor * image.size()));
    label->setPixmap(tmp);
}

void MainWindow::adjustMyScrollBars()
{
    ui->scrollArea_2->updateGeometry();
    ui->scrollArea_2->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value());
    ui->scrollArea_2->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value());
}

void MainWindow::slotOnlyLSBNormal(bool checked)
{
    if (!checked) return;

    resetImages(Utils::COLOR_NONE);
    adjustMySize(mModifiedImg, ui->ImageView);
    if (!mModifiedStego.isNull()) {
        adjustMySize(mModifiedStego, ui->ImageStegoView);
        adjustMyScrollBars();
    }

    mActions.at(0)->setChecked(false);
    mActions.at(1)->setChecked(false);
    mActions.at(2)->setChecked(false);
    mActions.at(3)->setChecked(false);
}

bool MainWindow::resetImages(Utils::Color color)
{
    if (mLastModified == Utils::COLOR_NONE) return true;

    if (mLastModified != color) {
        mModifiedImg = mImg;
        if (!mModifiedStego.isNull()) mModifiedStego = mStegosum->img();
        mLastModified = Utils::COLOR_NONE;
        return true;
    }

    return false;
}

void MainWindow::slotOnlyLSBAll(bool checked)
{
    if (!checked) return;

    if (resetImages(Utils::COLOR_ALL)) {
        if (convertToLSB(mModifiedImg, Utils::COLOR_ALL, ui->ImageView)) mLastModified = Utils::COLOR_ALL;
        if (convertToLSB(mModifiedStego, Utils::COLOR_ALL, ui->ImageStegoView)) adjustMyScrollBars();
    }

    mActions.at(0)->setChecked(false);
    mActions.at(1)->setChecked(false);
    mActions.at(2)->setChecked(false);
    mActions.at(4)->setChecked(false);
}

void MainWindow::slotOnlyLSBBlue(bool checked)
{
    if (!checked) return;

    if (resetImages(Utils::COLOR_BLUE)) {
        if (convertToLSB(mModifiedImg, Utils::COLOR_BLUE, ui->ImageView)) mLastModified = Utils::COLOR_BLUE;
        if (convertToLSB(mModifiedStego, Utils::COLOR_BLUE, ui->ImageStegoView)) adjustMyScrollBars();
    }

    mActions.at(0)->setChecked(false);
    mActions.at(1)->setChecked(false);
    mActions.at(3)->setChecked(false);
    mActions.at(4)->setChecked(false);
}

void MainWindow::slotOnlyLSBGreen(bool checked)
{
    if (!checked) return;

    if (resetImages(Utils::COLOR_GREEN)) {
        if (convertToLSB(mModifiedImg, Utils::COLOR_GREEN, ui->ImageView)) mLastModified = Utils::COLOR_GREEN;
        if (convertToLSB(mModifiedStego, Utils::COLOR_GREEN, ui->ImageStegoView)) adjustMyScrollBars();
    }

    mActions.at(0)->setChecked(false);
    mActions.at(2)->setChecked(false);
    mActions.at(3)->setChecked(false);
    mActions.at(4)->setChecked(false);
}

void MainWindow::slotOnlyLSBRed(bool checked)
{
    if (!checked) return;

    if (resetImages(Utils::COLOR_RED)){
        if (convertToLSB(mModifiedImg, Utils::COLOR_RED, ui->ImageView)) mLastModified = Utils::COLOR_RED;
        if (convertToLSB(mModifiedStego, Utils::COLOR_RED, ui->ImageStegoView)) adjustMyScrollBars();
    }

    mActions.at(1)->setChecked(false);
    mActions.at(2)->setChecked(false);
    mActions.at(3)->setChecked(false);
    mActions.at(4)->setChecked(false);
}

void MainWindow::slotImgMoveTool(QPoint p)
{
    if (!mFileName.isEmpty()) {
        ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() + p.x());
        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() + p.y());
    }

    if (!mModifiedStego.isNull()) {
        ui->scrollArea_2->horizontalScrollBar()->setValue(ui->scrollArea_2->horizontalScrollBar()->value() + p.x());
        ui->scrollArea_2->verticalScrollBar()->setValue(ui->scrollArea_2->verticalScrollBar()->value() + p.y());
    }
}

void MainWindow::slotChangeStegoImgVisib(bool change)
{
    if (change) {
        ui->scrollArea_2->show();
    } else {
        ui->scrollArea_2->hide();
    }
}

void MainWindow::on_makeAnalysisButton_clicked()
{
    if (mFileName.isEmpty()) return;

    ui->consoleOutput->setPlainText("");

    QString tmpFileName;
    if (ui->fileOutputcheckBox->isChecked()) {
        tmpFileName = QFileDialog::getSaveFileName(this, tr("Output filename"), mAnalysisOutFileName);

        if (!tmpFileName.isEmpty()) {
            mAnalysisOutFileName = tmpFileName;

            QFile file(tmpFileName);
            file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
            file.close();
        }
    }

    if (ui->samplePairsCheckBox->isChecked()) {
        SamplePairs sp;
        double avg = 0.0;
        QString outText;

        outText.append("\n------------------------------------------\nSample Pairs Analysis\n------------------------------------------\n");

        sp.analyse(mImg, Analysis::ANALYSIS_COLOR_RED);
        avg += sp.getMessageLength();
        outText.append("Percentage in red: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        sp.analyse(mImg, Analysis::ANALYSIS_COLOR_GREEN);
        avg += sp.getMessageLength();
        outText.append("Percentage in green: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        sp.analyse(mImg, Analysis::ANALYSIS_COLOR_BLUE);
        avg += sp.getMessageLength();
        outText.append("Percentage in blue: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        outText.append("Average percentage: " + QString::number((avg / 3) * 100) + "%\n");


        outText.append("\n------------------------------------------\nSample Pairs Analysis Old\n------------------------------------------\n");
        avg = 0.0;
        sp.analyseOld(mImg, Analysis::ANALYSIS_COLOR_RED);
        avg += sp.getMessageLength();
        outText.append("Percentage in red: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        sp.analyseOld(mImg, Analysis::ANALYSIS_COLOR_GREEN);
        avg += sp.getMessageLength();
        outText.append("Percentage in green: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        sp.analyseOld(mImg, Analysis::ANALYSIS_COLOR_BLUE);
        avg += sp.getMessageLength();
        outText.append("Percentage in blue: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        outText.append("Average percentage: " + QString::number((avg / 3) * 100) + "%\n");

        if (ui->consoleOutputCheckBox->isChecked()) {
            ui->consoleOutput->appendPlainText(outText);
        }

        if (!tmpFileName.isEmpty()) {
            QFile file(mAnalysisOutFileName);
            file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
            QTextStream out(&file);
            out << outText;
            file.close();
        }
    }

    if (ui->checkBoxRS->isChecked()) {
        RS rs;
        double avg;
        avg = 0;
        QString outText;

        outText.append("\n------------------------------------------\nRS Analysis\n------------------------------------------\n");

        outText.append("\n(non-overlapping groups)\n");

        rs.analyse(mImg, Analysis::ANALYSIS_COLOR_RED, false);
        avg += rs.getMessageLength();
        outText.append("Percentage in red: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        rs.analyse(mImg, Analysis::ANALYSIS_COLOR_GREEN, false);
        avg += rs.getMessageLength();
        outText.append("Percentage in green: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        rs.analyse(mImg, Analysis::ANALYSIS_COLOR_BLUE, false);
        avg += rs.getMessageLength();
        outText.append("Percentage in blue: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        outText.append("\n(overlapping groups)\n");

        rs.analyse(mImg, Analysis::ANALYSIS_COLOR_RED, true);
        avg += rs.getMessageLength();
        outText.append("Percentage in red: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        rs.analyse(mImg, Analysis::ANALYSIS_COLOR_GREEN, true);
        avg += rs.getMessageLength();
        outText.append("Percentage in green: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        rs.analyse(mImg, Analysis::ANALYSIS_COLOR_BLUE, true);
        avg += rs.getMessageLength();
        outText.append("Percentage in blue: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        outText.append("\nAverage percentage: " + QString::number((avg / 6) * 100) + "%\n");

        if (ui->consoleOutputCheckBox->isChecked()) {
            ui->consoleOutput->appendPlainText(outText);
        }

        if (!tmpFileName.isEmpty()) {
            QFile file(mAnalysisOutFileName);
            file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
            QTextStream out(&file);
            out << outText;
            file.close();
        }
    }
}

void MainWindow::slotNormalSize()
{
    QPixmap tmp, tmps;

    tmp.convertFromImage(mModifiedImg);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(mModifiedStego)) ui->ImageStegoView->setPixmap(tmps);

    mScaleFactor = 1.0f;
}

void MainWindow::slotZoomIn() {slotZoom(1);}
void MainWindow::slotZoomOut() {slotZoom(-1);}
void MainWindow::slotZoom(int scrollDelta)
{
    if (mFileName.isEmpty()) return;

    if (scrollDelta > 0) {
        if (mScaleFactor > 3.0) return;
        scaleImage(1.25);
    } else {
        if (mScaleFactor < 0.333) return;
        scaleImage(0.8);
    }

    actionZoom_In_25->setEnabled(mScaleFactor < 3.0);
    actionZoom_Out_25->setEnabled(mScaleFactor > 0.333);
}

void MainWindow::scaleImage(float factor)
{
    QPixmap tmp, tmps;

    mScaleFactor *= factor;
    ui->statusBar->showMessage(QString().sprintf("Zoom %.0f%%", mScaleFactor * 100), 2000);

    QSize size = mScaleFactor * mModifiedImg.size();

//    tmp.convertFromImage(mModifiedImg.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    tmp.convertFromImage(mModifiedImg.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation));
    ui->ImageView->setPixmap(tmp);

//    if (tmp.convertFromImage(mModifiedStego.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)))
    if (tmps.convertFromImage(mModifiedStego.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation)))
        ui->ImageStegoView->setPixmap(tmps);

//    ui->scrollArea->horizontalScrollBar()->setValue(
//                int(mScaleFactor * ui->scrollArea->horizontalScrollBar()->value()
//                    + ((mScaleFactor - 1) * ui->scrollArea->horizontalScrollBar()->pageStep()/2)));

//    ui->scrollArea->verticalScrollBar()->setValue(
//                int(mScaleFactor * ui->scrollArea->verticalScrollBar()->value()
//                    + ((mScaleFactor - 1) * ui->scrollArea->verticalScrollBar()->pageStep()/2)));
}

MainWindow::~MainWindow()
{
    delete ui;
    if (mStegosum) delete mStegosum;
}

void MainWindow::setProgress(int val)
{
    mPgBar->setValue(mPgBar->value() + val);
}

/*! 
 Encrypts the secret message bytes using aes128 block cipher
 and prepends 16 bytes initialisation vector before the
 secret message.

 @param  msg  secret message bytes
 @return  encrypted secret message
 @return asdfomgnon
 */
QByteArray MainWindow::encrypt(QByteArray msg)
{
    QCA::SymmetricKey key = QCA::SymmetricKey(mPassword.toAscii());
    QCA::InitializationVector iv = QCA::InitializationVector(16);

    mCipher.setup(QCA::Encode, key, iv);

    QByteArray ret = iv.toByteArray();
    ret += mCipher.process(msg).toByteArray();
    return ret;
}

QByteArray MainWindow::decrypt(QByteArray msg)
{
    QCA::SymmetricKey key = QCA::SymmetricKey(mPassword.toAscii());

    if (mPassword.isEmpty()) return QByteArray();

    QCA::InitializationVector iv(msg.left(16));

    mCipher.setup(QCA::Decode, key, iv);

    return mCipher.process(msg.mid(16)).toByteArray();
}

void MainWindow::compressSliderChangedHandler(int change)
{
    if (change == 9) ui->compressLabelChange->setText("max");
    else if (change == 0) ui->compressLabelChange->setText("none");
    else ui->compressLabelChange->setText(QString::number(change));

    if (change)
    {
        mSecretBytes = qCompress(mUnchangedSecretBytes, change);

        if (ui->encryptCheckBox->isChecked())
        {
            mSecretBytes = encrypt(mSecretBytes);
        }
    }
    else
    {
        if (ui->encryptCheckBox->isChecked())
        {
            mSecretBytes = encrypt(mUnchangedSecretBytes);
        }
        else
        {
            mSecretBytes = mUnchangedSecretBytes;
        }
    }

    updateStatusBar();
}

void MainWindow::setNumMax()
{
    mColors.set(ui->red_radio->isChecked(), ui->green_radio->isChecked(), ui->blue_radio->isChecked());

    mNumWritableChars = ((mImg.width() * mImg.height() - SUFFLEEOFFSET - NUM_OF_SETTINGS_PIXELS - Utils::pixelsNeeded(NUM_OF_SIZE_BITS, mColors.numOfselected)) * mColors.numOfselected) / 8 ;
}


void MainWindow::on_red_radio_clicked(bool checked)
{
    Q_UNUSED(checked);
    setNumMax();
    updateStatusBar();
}

void MainWindow::on_green_radio_clicked(bool checked)
{
    Q_UNUSED(checked);
    setNumMax();
    updateStatusBar();
}

void MainWindow::on_blue_radio_clicked(bool checked)
{
    Q_UNUSED(checked);
    setNumMax();
    updateStatusBar();
}

void MainWindow::on_lookAheadRadio_clicked(bool checked)
{
    ui->red_radio->setEnabled(!checked);
    ui->green_radio->setEnabled(!checked);
    ui->blue_radio->setEnabled(!checked);
    updateStatusBar();
}

void MainWindow::on_encryptCheckBox_clicked()
{
    if (ui->encryptCheckBox->isChecked())
    {

        QByteArray codeArray;
        if (ui->compressSlider->value())
        {
            codeArray = mSecretBytes;
        }
        else
        {
            codeArray = mUnchangedSecretBytes;
        }

        mSecretBytes = encrypt(codeArray);
    }
    else
    {
        if (ui->compressSlider->value())
        {
            mSecretBytes = qCompress(mUnchangedSecretBytes, ui->compressSlider->value());
        }
        else
        {
            mSecretBytes = mUnchangedSecretBytes;
        }
    }

    updateStatusBar();
}

void MainWindow::secretMsgTextChangedHandler()
{
    if (!mImg.isNull()) ui->encodeButton->setEnabled(true);

    mUnchangedSecretBytes = ui->secretMsgView->toPlainText().toAscii();
    mSecretBytes = mUnchangedSecretBytes;

    if (ui->compressSlider->value())
    {
        mSecretBytes = qCompress(mUnchangedSecretBytes, ui->compressSlider->value());
    }

    if (ui->encryptCheckBox->isChecked())
    {
        mSecretBytes = encrypt(mSecretBytes);
    }

    if (mUnchangedSecretBytes.isEmpty()) ui->encodeButton->setEnabled(false);

    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    int size = mSecretBytes.size();

    if (mImg.isNull()) {
        ui->statusBar->showMessage(QString::number(size)  + " characters", 2000);
        return;
    }

    if (ui->lookAheadRadio->isChecked()) {
        ui->statusBar->showMessage(QString::number(size) + " / ? characters", 2000);
        return;
    }

    ui->statusBar->showMessage(QString::number(size) + " / " + QString::number(mNumWritableChars) + " characters", 2000);
}

void MainWindow::secretMsgTextEditPressHandler()
{
    on_actionEncode_triggered();
}

void MainWindow::receiveSecretMsg(QByteArray msgBytes, bool compressed, bool encrypted)
{
    if (encrypted) {
        msgBytes = decrypt(msgBytes);
        if (msgBytes.isEmpty()) {
            statusBar()->showMessage(tr("Error while decoding."), 2000);
            slotWriteToConsole("[MainWindow] Image probably has encrypted secret message in it. Wrong password.\n");
            return;
        }
    }

    if (compressed) {
        msgBytes = qUncompress(msgBytes);
    }

    if (ui->radioButtonText->isChecked()) {
        ui->secretMsgView->setPlainText(QString(msgBytes));
    } else {
        QString saveDataFileName = QFileDialog::getSaveFileName(this, tr("Save Image"), mFileName);

        if (!saveDataFileName.isEmpty()) {

            QFile dataFile(saveDataFileName);

            if (!dataFile.open(QIODevice::WriteOnly)) {
                ui->statusBar->showMessage("Cannot save data file.", 2000);
                return;
            }

            dataFile.write(msgBytes);
            dataFile.close();

        }
    }
}

void MainWindow::on_actionOpen_triggered()
{    
    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open Image"), mFileName, tr("Image Files (*.png *.bmp *.svg)"));

    if (!tmpFileName.isEmpty()) {

        openImage(tmpFileName);
        mScaleFactor = 1.0f;
        mLastModified = Utils::COLOR_NONE;
    }
}

void MainWindow::openImage(QString name)
{
    QPixmap tmp;

    mFileName = name;

    mImg.load(mFileName);

    if (mStegosum) delete mStegosum;

    QAction * action;
    QByteArray format = QImageReader(mFileName).format();
    if (format == "bmp" || format == "png") {
        ui->stackedWidget->setCurrentIndex(1);
        mStegosum = new Raster();

        foreach (QAction * a, mActions) {
            mMenuView->removeAction(a);
        }
        mActions.clear();

        action = mMenuView->addAction("Only LSB &Red");
        action->setCheckable(true);
        connect(action, SIGNAL(toggled(bool)), this, SLOT(slotOnlyLSBRed(bool)));
        mActions.append(action);

        action = mMenuView->addAction("Only LSB &Green");
        action->setCheckable(true);
        connect(action, SIGNAL(toggled(bool)), this, SLOT(slotOnlyLSBGreen(bool)));
        mActions.append(action);

        action = mMenuView->addAction("Only LSB &Blue");
        action->setCheckable(true);
        connect(action, SIGNAL(toggled(bool)), this, SLOT(slotOnlyLSBBlue(bool)));
        mActions.append(action);

        action = mMenuView->addAction("Only LSB &All");
        action->setCheckable(true);
        connect(action, SIGNAL(toggled(bool)), this, SLOT(slotOnlyLSBAll(bool)));
        mActions.append(action);

        action = mMenuView->addAction("LSB &Normal View");
        action->setCheckable(true);
        action->setChecked(true);
        connect(action, SIGNAL(toggled(bool)), this, SLOT(slotOnlyLSBNormal(bool)));
        mActions.append(action);

    } else if (format == "svg") {
        ui->stackedWidget->setCurrentIndex(2);
        mStegosum = new Vector();

        foreach (QAction * a, mActions) {
            mMenuView->removeAction(a);
        }
        mActions.clear();
    }

    mStegosum->setImageName(name);

    connect(mStegosum, SIGNAL(succes(bool)), this, SLOT(threadEncodeHandler(bool)));
    connect(mStegosum, SIGNAL(setMaximum(int)), mPgBar, SLOT(setMaximum(int)));
    connect(mStegosum, SIGNAL(sendMessage(QByteArray,bool,bool)), this, SLOT(receiveSecretMsg(QByteArray,bool,bool)));
    connect(mStegosum, SIGNAL(updateProgress(int)), this, SLOT(setProgress(int)));
    connect(mStegosum, SIGNAL(writeToConsole(QString)), this, SLOT(slotWriteToConsole(QString)));

    mModifiedImg = mImg;

    tmp.convertFromImage(mModifiedImg);
    ui->ImageView->setPixmap(tmp);

    ui->ImageStegoView->setPixmap(0);
    ui->ImageStegoView->setText("stego image");
    mModifiedStego = QImage();

    ui->decodeButton->setEnabled(true);

    ui->makeAnalysisButton->setEnabled(true);

    ui->compressSlider->setEnabled(true);

    actionZoom_In_25->setEnabled(true);

    actionZoom_Out_25->setEnabled(true);

    action_Normal_Size->setEnabled(true);

    if (!mPassword.isEmpty()) ui->encryptCheckBox->setEnabled(true);

    ui->lookAheadRadio->setEnabled(true);
    ui->metaCheckBox->setEnabled(true);

    ui->red_radio->setEnabled(true);
    ui->green_radio->setEnabled(true);
    ui->blue_radio->setEnabled(true);

    if (ui->radioButtonText->isChecked()) {
        if (!ui->secretMsgView->toPlainText().isEmpty()) ui->encodeButton->setEnabled(true);
    } else {
        if (!mDataFileName.isEmpty()) ui->encodeButton->setEnabled(true);
    }

    setNumMax();
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    mPassword = arg1;
    if (!mPassword.isEmpty() && !mImg.isNull()) ui->encryptCheckBox->setEnabled(true);
    if (mPassword.isEmpty())
    {
        ui->encryptCheckBox->setEnabled(false);
        ui->encryptCheckBox->setChecked(false);
    }
}

void MainWindow::threadEncodeHandler(bool succ)
{
    if (!succ) {

        if (mStegosum->isEncode()) statusBar()->showMessage(tr("Error while encoding."), 2000);
        else statusBar()->showMessage(tr("Error while decoding."), 2000);

    } else if (mStegosum->isEncode()) {
        QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save Image"), mFileName, tr("Image Files (*.png *.bmp *.svg)"));

        if (!saveFileName.isEmpty()) {
            mStegosum->save(saveFileName);

            mModifiedStego = mStegosum->img();

            if (!convertToLSB(mModifiedStego, mLastModified, ui->ImageStegoView)) {
                adjustMySize(mModifiedStego, ui->ImageStegoView);
            }

            ui->scrollArea_2->adjustSize();
            ui->scrollArea_2->horizontalScrollBar()->setMaximum(ui->scrollArea->horizontalScrollBar()->maximum());

            adjustMyScrollBars();
        }
    }

    ui->encodeButton->setEnabled(true);
    ui->decodeButton->setEnabled(true);
    mPgBar->reset();
    mPgBar->setVisible(false);
}

void MainWindow::on_actionEncode_triggered()
{
    mPgBar->setMinimum(0);
    mPgBar->setVisible(true);

    ui->encodeButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);

    mStegosum->setUp(this, true, (ui->compressSlider->value()) ? true : false,
                     ui->encryptCheckBox->isChecked(),
                     ui->lookAheadRadio->isChecked(),
                     ui->metaCheckBox->isChecked(),
                     ui->FPPosSlider->value(),
                     ui->checkBoxMaxFPPosition->isChecked());
    mStegosum->start();
}

void MainWindow::on_actionDecode_triggered()
{
    mPgBar->setMinimum(0);
    mPgBar->setVisible(true);

    ui->encodeButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);

    mStegosum->setUp(this, false);
    mStegosum->start();
}

void MainWindow::on_radioButtonData_clicked()
{
    ui->secretMsgView->setEnabled(false);
    ui->openDataButton->setEnabled(true);
}


void MainWindow::on_radioButtonText_clicked()
{
    ui->secretMsgView->setEnabled(true);
    ui->openDataButton->setEnabled(false);
}

void MainWindow::on_openDataButton_clicked()
{
    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open Data"), mDataFileName);

    if (!tmpFileName.isEmpty()) {

        mDataFileName = tmpFileName;

        QFile dataFile(mDataFileName);

        if (!dataFile.open(QIODevice::ReadOnly))
        {
            ui->statusBar->showMessage("Cannot open data file.", 2000);
            mDataFileName.clear();
            return;
        }
        else
        {
            ui->statusBar->showMessage("Data loaded.", 2000);
        }

        mUnchangedSecretBytes = dataFile.readAll();
        mSecretBytes = mUnchangedSecretBytes;

        if (!mImg.isNull())
        {
            ui->encodeButton->setEnabled(true);

            ui->compressSlider->setEnabled(true);

            if (!mPassword.isEmpty()) ui->encryptCheckBox->setEnabled(true);

            ui->red_radio->setEnabled(true);
            ui->green_radio->setEnabled(true);
            ui->blue_radio->setEnabled(true);
        }

        dataFile.close();
    }
}

void MainWindow::slotWriteToConsole(QString string) {
    ui->console->append(string);
}

void MainWindow::on_FPPosSlider_sliderMoved(int position) {
    ui->FPPosLabel->setText(QString::number(Stegosum::streamToReal(QString("12345678"), position), 'g', 8));
}

void MainWindow::on_checkBoxMaxFPPosition_toggled(bool checked)
{
    ui->FPPosSlider->setEnabled(!checked);
    ui->FPPosLabel->setEnabled(!checked);
}
