#include <QImageReader>
#include <QImageWriter>
#include <QtSvg>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "analysis/samplepairs.h"
#include "analysis/rs.h"
#include "utils.hpp"
#include "raster.h"
#include "vector.h"
#include "variation.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mStegosum(0),
    mNumWritableChars(0),
    mInit(),
    mScaleFactor(1.0),
    mIsDebug(false)
{
    ui->setupUi(this);

    mPgBar = new QProgressBar(this);
    mPgBar->setVisible(false);

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

    //FIXME1
    if (mIsDebug) {
        ui->stackedWidget->setCurrentIndex(2);
        mStegosum = new Vector("");
        ui->checkBoxMaxFPPosition->setChecked(true);
        ui->encodeButton->setEnabled(true);
        ui->decodeButton->setEnabled(true);
    }
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

    mStegosum->setSelectedImgs(Utils::COLOR_NONE);

    QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

    QPixmap tmp, tmps;

    tmp.convertFromImage(pair.first);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(pair.second))
        ui->ImageStegoView->setPixmap(tmps);

    mActions.at(0)->setChecked(false);
    mActions.at(1)->setChecked(false);
    mActions.at(2)->setChecked(false);
    mActions.at(3)->setChecked(false);
}

void MainWindow::slotOnlyLSBAll(bool checked)
{
    if (!checked) return;

    mStegosum->setSelectedImgs(Utils::COLOR_ALL);

    QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

    QPixmap tmp, tmps;

    tmp.convertFromImage(pair.first);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(pair.second))
        ui->ImageStegoView->setPixmap(tmps);

    mActions.at(0)->setChecked(false);
    mActions.at(1)->setChecked(false);
    mActions.at(2)->setChecked(false);
    mActions.at(4)->setChecked(false);
}

void MainWindow::slotOnlyLSBBlue(bool checked)
{
    if (!checked) return;

    mStegosum->setSelectedImgs(Utils::COLOR_BLUE);

    QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

    QPixmap tmp, tmps;

    tmp.convertFromImage(pair.first);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(pair.second))
        ui->ImageStegoView->setPixmap(tmps);

    mActions.at(0)->setChecked(false);
    mActions.at(1)->setChecked(false);
    mActions.at(3)->setChecked(false);
    mActions.at(4)->setChecked(false);
}

void MainWindow::slotOnlyLSBGreen(bool checked)
{
    if (!checked) return;

    mStegosum->setSelectedImgs(Utils::COLOR_GREEN);

    QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

    QPixmap tmp, tmps;

    tmp.convertFromImage(pair.first);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(pair.second))
        ui->ImageStegoView->setPixmap(tmps);

    mActions.at(0)->setChecked(false);
    mActions.at(2)->setChecked(false);
    mActions.at(3)->setChecked(false);
    mActions.at(4)->setChecked(false);
}

void MainWindow::slotOnlyLSBRed(bool checked)
{
    if (!checked) return;

    mStegosum->setSelectedImgs(Utils::COLOR_RED);

    QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

    QPixmap tmp, tmps;

    tmp.convertFromImage(pair.first);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(pair.second))
        ui->ImageStegoView->setPixmap(tmps);

    mActions.at(1)->setChecked(false);
    mActions.at(2)->setChecked(false);
    mActions.at(3)->setChecked(false);
    mActions.at(4)->setChecked(false);
}

void MainWindow::slotImgMoveTool(QPoint p)
{
    if (ui->ImageView->pixmap()) {
        ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() + p.x());
        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() + p.y());
    }

    if (ui->ImageStegoView->pixmap()) {
        ui->scrollArea_2->horizontalScrollBar()->setValue(ui->scrollArea_2->horizontalScrollBar()->value() + p.x());
        ui->scrollArea_2->verticalScrollBar()->setValue(ui->scrollArea_2->verticalScrollBar()->value() + p.y());
    }
}

void MainWindow::slotChangeStegoImgVisib(bool change)
{
    if (change)
        ui->scrollArea_2->show();
    else
        ui->scrollArea_2->hide();
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

        sp.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_RED);
        avg += sp.getMessageLength();
        outText.append("Percentage in red: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        sp.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_GREEN);
        avg += sp.getMessageLength();
        outText.append("Percentage in green: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        sp.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_BLUE);
        avg += sp.getMessageLength();
        outText.append("Percentage in blue: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        outText.append("Average percentage: " + QString::number((avg / 3) * 100) + "%\n");


        outText.append("\n------------------------------------------\nSample Pairs Analysis Old\n------------------------------------------\n");
        avg = 0.0;
        sp.analyseOld(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_RED);
        avg += sp.getMessageLength();
        outText.append("Percentage in red: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        sp.analyseOld(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_GREEN);
        avg += sp.getMessageLength();
        outText.append("Percentage in green: " + QString::number(sp.getMessageLength() * 100) + "%\n");

        sp.analyseOld(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_BLUE);
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

        rs.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_RED, false);
        avg += rs.getMessageLength();
        outText.append("Percentage in red: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        rs.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_GREEN, false);
        avg += rs.getMessageLength();
        outText.append("Percentage in green: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        rs.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_BLUE, false);
        avg += rs.getMessageLength();
        outText.append("Percentage in blue: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        outText.append("\n(overlapping groups)\n");

        rs.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_RED, true);
        avg += rs.getMessageLength();
        outText.append("Percentage in red: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        rs.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_GREEN, true);
        avg += rs.getMessageLength();
        outText.append("Percentage in green: " + QString::number(rs.getMessageLength() * 100) + "%\n");

        rs.analyse(mStegosum->getImgs().first, Analysis::ANALYSIS_COLOR_BLUE, true);
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

    mScaleFactor = 1.0f;

    QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

    tmp.convertFromImage(pair.first);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(pair.second))
        ui->ImageStegoView->setPixmap(tmps);
}

void MainWindow::slotZoomIn() {slotZoom(1);}
void MainWindow::slotZoomOut() {slotZoom(-1);}
void MainWindow::slotZoom(int scrollDelta)
{
    if (!ui->ImageView->pixmap()) return;

    if (scrollDelta > 0) {
        if (mScaleFactor > 7.0) return;
        scaleImage(1.25);
    } else {
        if (mScaleFactor < 0.2) return;
        scaleImage(0.8);
    }

    actionZoom_In_25->setEnabled(mScaleFactor < 7.0);
    actionZoom_Out_25->setEnabled(mScaleFactor > 0.2);
}

void MainWindow::scaleImage(float factor)
{
    QPixmap tmp, tmps;

    mScaleFactor *= factor;
    ui->statusBar->showMessage(QString().sprintf("Zoom %.0f%%", mScaleFactor * 100), 2000);

    QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

    tmp.convertFromImage(pair.first);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(pair.second))
        ui->ImageStegoView->setPixmap(tmps);
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
            mSecretBytes = Utils::encrypt(mSecretBytes, mPassword);
        }
    }
    else
    {
        if (ui->encryptCheckBox->isChecked())
        {
            mSecretBytes = Utils::encrypt(mUnchangedSecretBytes, mPassword);
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

    mNumWritableChars = ((mStegosum->getImgs().first.width() * mStegosum->getImgs().first.height() - Utils::SUFFLEEOFFSET - Utils::NUM_OF_SETTINGS_PIXELS - Utils::pixelsNeeded(Utils::NUM_OF_SIZE_BITS, mColors.numOfselected)) * mColors.numOfselected) / 8 ;
}


void MainWindow::on_red_radio_clicked(bool checked)
{
    if (!checked && !ui->green_radio->isChecked() && !ui->blue_radio->isChecked()) {
        ui->red_radio->setChecked(true);
        return;
    }
    setNumMax();
    updateStatusBar();
}

void MainWindow::on_green_radio_clicked(bool checked)
{
    if (!checked && !ui->red_radio->isChecked() && !ui->blue_radio->isChecked()) {
        ui->green_radio->setChecked(true);
        return;
    }
    setNumMax();
    updateStatusBar();
}

void MainWindow::on_blue_radio_clicked(bool checked)
{
    if (!checked && !ui->green_radio->isChecked() && !ui->red_radio->isChecked()) {
        ui->blue_radio->setChecked(true);
        return;
    }
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

        mSecretBytes = Utils::encrypt(codeArray, mPassword);
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
    if (ui->ImageView->pixmap()) ui->encodeButton->setEnabled(true);

    mUnchangedSecretBytes = ui->secretMsgView->toPlainText().toAscii();
    mSecretBytes = mUnchangedSecretBytes;

    if (ui->compressSlider->value())
    {
        mSecretBytes = qCompress(mUnchangedSecretBytes, ui->compressSlider->value());
    }

    if (ui->encryptCheckBox->isChecked())
    {
        mSecretBytes = Utils::encrypt(mSecretBytes, mPassword);
    }

    if (mUnchangedSecretBytes.isEmpty()) ui->encodeButton->setEnabled(false);

    updateStatusBar();
}

void MainWindow::updateStatusBar()
{
    int size = mSecretBytes.size();

    if (!ui->ImageView->pixmap()) {
        ui->statusBar->showMessage(QString::number(size)  + " characters", 2000);
        return;
    }

    if (ui->lookAheadRadio->isChecked() || !mStegosum->isRaster()) {
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

//        msgBytes = decrypt(msgBytes);
        msgBytes = Utils::decrypt(msgBytes, mPassword);

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
    }
}

void MainWindow::openImage(QString name)
{
    QPixmap tmp;

    mFileName = name;

    if (mStegosum) delete mStegosum;

    QAction * action;
    QByteArray format = QImageReader(mFileName).format();
    if (format == "bmp" || format == "png") {
        ui->stackedWidget->setCurrentIndex(1);
        mStegosum = new Raster(name);

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

        ui->makeAnalysisButton->setEnabled(true);

    } else if (format == "svg") {
        ui->stackedWidget->setCurrentIndex(2);
        mStegosum = new Vector(name);

        foreach (QAction * a, mActions) {
            mMenuView->removeAction(a);
        }
        mActions.clear();

        action = mMenuView->addAction("View &Points");
        action->setCheckable(true);
        connect(action, SIGNAL(toggled(bool)), this, SLOT(slotViewPoints(bool)));
        mActions.append(action);

        ui->makeAnalysisButton->setEnabled(false);
    }

    connect(mStegosum, SIGNAL(succes(bool)), this, SLOT(threadEncodeHandler(bool)));
    connect(mStegosum, SIGNAL(setMaximum(int)), mPgBar, SLOT(setMaximum(int)));
    connect(mStegosum, SIGNAL(sendMessage(QByteArray,bool,bool)), this, SLOT(receiveSecretMsg(QByteArray,bool,bool)));
    connect(mStegosum, SIGNAL(updateProgress(int)), this, SLOT(setProgress(int)));
    connect(mStegosum, SIGNAL(writeToConsole(QString)), this, SLOT(slotWriteToConsole(QString)));
    connect(mStegosum, SIGNAL(writeToStatus(QString)), this, SLOT(slotWriteToStatus(QString)));

    ui->ImageView->setPixmap(QPixmap(name));

    ui->ImageStegoView->setPixmap(0);
    ui->ImageStegoView->setText("stego image");

    ui->decodeButton->setEnabled(true);

    ui->compressSlider->setEnabled(true);

    actionZoom_In_25->setEnabled(true);

    actionZoom_Out_25->setEnabled(true);

    action_Normal_Size->setEnabled(true);

    if (!mPassword.isEmpty()) ui->encryptCheckBox->setEnabled(true);

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
    if (!mPassword.isEmpty() && ui->ImageView->pixmap()) ui->encryptCheckBox->setEnabled(true);
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
            mStegosum->saveStegoImg(saveFileName);

            mStegosum->setSelectedImgs(Utils::COLOR_PREV);

            QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

            QPixmap tmp;
            tmp.convertFromImage(pair.second);
            ui->ImageStegoView->setPixmap(tmp);

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

    //FIXME TESTING LOOKAHEAD SECRET MESSAGE CAPACITY
    /*QString alphabet = "abcdefghijklmnopqrstuvwxyz";

    mSecretBytes = "a";
    //mSecretBytes = QByteArray(1, ch.toAscii());
    mColors.set(true, true, true);

    Stegosum * sum;
    foreach(const QChar ch, alphabet) {
        mPassword = ch;
        sum = new Raster("/home/evilbateye/develop/CD/stegosum-build-desktop-Qt_4_8_3_in_PATH__System__Release/kitty_.png");
        sum->setUp(this, true, false, false, true, 0, false);
        sum->start();
    }*/

    mStegosum->setUp(this, true, (ui->compressSlider->value()) ? true : false,
                 ui->encryptCheckBox->isChecked(),
                 ui->lookAheadRadio->isChecked(),
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

        if (ui->ImageView->pixmap())
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

void MainWindow::slotWriteToStatus(QString string) {
    ui->statusBar->showMessage(string, 2000);
}
void MainWindow::on_FPPosSlider_sliderMoved(int position) {
    ui->FPPosLabel->setText(QString::number(Vector::streamToReal(QString("12345678"), position), 'g', 8));
}

void MainWindow::on_checkBoxMaxFPPosition_toggled(bool checked)
{
    ui->FPPosSlider->setEnabled(!checked);
    ui->FPPosLabel->setEnabled(!checked);
}

void MainWindow::slotViewPoints(bool checked) {
    if (checked) mStegosum->setSelectedImgs(Utils::COLOR_ILUM);
    else mStegosum->setSelectedImgs(Utils::COLOR_NONE);

    QPair<QImage, QImage> pair = mStegosum->scaleImgs(mScaleFactor);

    QPixmap tmp, tmps;

    tmp.convertFromImage(pair.first);
    ui->ImageView->setPixmap(tmp);

    if (tmps.convertFromImage(pair.second))
        ui->ImageStegoView->setPixmap(tmps);
}
