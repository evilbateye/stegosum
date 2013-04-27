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
#include "clickablelabel.hpp"
#include "analysis/samplepairs.h"


#define SUFFLEEOFFSET 2
#define SIZE_ENCODE_OFFSET 24
#define SETTINGS_OFFSET 3


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mNumWritableChars(0),
    mInit(),
    mCipher(QString("aes128"), QCA::Cipher::CBC, QCA::Cipher::DefaultPadding),
    mScaleFactor(1.0)
{
    ui->setupUi(this);

    mPgBar = new QProgressBar(this);
    mPgBar->setVisible(false);
    //ui->statusBar->addPermanentWidget(mPgBar);

    connect(ui->secretMsgView, SIGNAL(textChanged()), this, SLOT(secretMsgTextChangedHandler()));
    connect(ui->compressSlider, SIGNAL(valueChanged(int)), this, SLOT(compressSliderChangedHandler(int)));

    connect(&mPointGenThread, SIGNAL(succes(bool)), this, SLOT(threadEncodeHandler(bool)));
    connect(&mPointGenThread, SIGNAL(setMaximum(int)), mPgBar, SLOT(setMaximum(int)));
    connect(&mPointGenThread, SIGNAL(sendMessage(QByteArray,bool,bool)), this, SLOT(receiveSecretMsg(QByteArray,bool,bool)));
    connect(&mPointGenThread, SIGNAL(updateProgress(int)), this, SLOT(setProgress(int)));

    connect(ui->ImageView, SIGNAL(clicked()), this, SLOT(on_actionOpen_triggered()));
    connect(ui->ImageView, SIGNAL(scrolled(int)), this, SLOT(slotZoom(int)));

    //IMAGE VIEW MANIPULATION
    //ui->ImageView->setBackgroundRole(QPalette::Base);
    //ui->ImageView->setScaledContents(true);
    ui->scrollArea->setWidgetResizable(true);
    //ui->ImageView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

//    ui->scrollArea_2->hide();
    connect(ui->action_Stego_Image, SIGNAL(toggled(bool)), this, SLOT(slotChangeStegoImgVisib(bool)));

    connect(ui->actionZoom_In_25, SIGNAL(triggered()), this, SLOT(slotZoomIn()));
    connect(ui->actionZoom_Out_25, SIGNAL(triggered()), this, SLOT(slotZoomOut()));
    connect(ui->action_Normal_Size, SIGNAL(triggered()), this, SLOT(slotNormalSize()));
    connect(ui->action_New, SIGNAL(triggered()), this, SLOT(on_actionOpen_triggered()));
    connect(ui->ImageView, SIGNAL(mousePressedAndMoved(QPoint)), this, SLOT(slotImgMoveTool(QPoint)));
    connect(ui->ImageStegoView, SIGNAL(mousePressedAndMoved(QPoint)), this, SLOT(slotImgMoveTool(QPoint)));

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
}

void MainWindow::slotImgMoveTool(QPoint p)
{
    if (!mFileName.isEmpty()) {
        ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() + p.x());
        ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() + p.y());
    }

    if (!mViewStegoPixmap.isNull()) {
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
        double avg, result;
        avg = result = 0;
        QString outText;

        outText.append("\n------------------------------------------\nSample Pairs Analysis\n------------------------------------------\n");

        result = sp.analyse(mImg, Analysis::ANALYSIS_COLOR_RED);
        avg += result;
        outText.append("Percentage in red: " + QString::number(result * 100) + "%\n");

        result = sp.analyse(mImg, Analysis::ANALYSIS_COLOR_GREEN);
        avg += result;
        outText.append("Percentage in green: " + QString::number(result * 100) + "%\n");

        result = sp.analyse(mImg, Analysis::ANALYSIS_COLOR_BLUE);
        avg += result;
        outText.append("Percentage in blue: " + QString::number(result * 100) + "%\n");

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
}

void MainWindow::slotNormalSize()
{
    ui->ImageView->setPixmap(mViewPixmap);
    if (!mViewStegoPixmap.isNull()) ui->ImageStegoView->setPixmap(mViewStegoPixmap);
    mScaleFactor = 1.0f;
}

void MainWindow::slotZoomIn() {slotZoom(1);}
void MainWindow::slotZoomOut() {slotZoom(-1);}
void MainWindow::slotZoom(int scrollDelta)
{
    if (scrollDelta > 0) {
        if (mScaleFactor > 3.0) return;
        scaleImage(1.25);
    } else {
        if (mScaleFactor < 0.333) return;
        scaleImage(0.8);
    }

    ui->actionZoom_In_25->setEnabled(mScaleFactor < 3.0);
    ui->actionZoom_Out_25->setEnabled(mScaleFactor > 0.333);
}

void MainWindow::scaleImage(float factor)
{
    mScaleFactor *= factor;
    ui->statusBar->showMessage(QString().sprintf("Zoom %.0f%%", mScaleFactor * 100), 2000);

    QSize size = mScaleFactor * mViewPixmap.size();
    ui->ImageView->setPixmap(mViewPixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    if (!mViewStegoPixmap.isNull()) ui->ImageStegoView->setPixmap(mViewStegoPixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));


    /*ui->scrollArea->horizontalScrollBar()->setValue(
                int(mScaleFactor * ui->scrollArea->horizontalScrollBar()->value() +
                    ((mScaleFactor - 1) * ui->scrollArea->horizontalScrollBar()->pageStep()/2)));*/

    /*ui->scrollArea->verticalScrollBar()->setValue(
                int(mScaleFactor * ui->scrollArea->verticalScrollBar()->value() +
                    ((mScaleFactor - 1) * ui->scrollArea->verticalScrollBar()->pageStep()/2)));*/
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setNumMax()
{
    if (!ui->encodeMaxCheckBox->isChecked())
    {
        mNumWritableChars = (mImg.width() * mImg.height() - SUFFLEEOFFSET - SIZE_ENCODE_OFFSET - 1) / 8;
    }
    else
    {
        mNumWritableChars = ((mImg.width() * mImg.height() - 1 - SUFFLEEOFFSET) * 3 - SIZE_ENCODE_OFFSET) / 8;
    }
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

void MainWindow::on_encodeMaxCheckBox_clicked()
{
    setNumMax();

    if (ui->encodeMaxCheckBox->isChecked()) {
        ui->red_radio->setEnabled(false);
        ui->green_radio->setEnabled(false);
        ui->blue_radio->setEnabled(false);
        ui->all_radio->setEnabled(false);
    } else {
        ui->red_radio->setEnabled(true);
        ui->green_radio->setEnabled(true);
        ui->blue_radio->setEnabled(true);
        ui->all_radio->setEnabled(true);
    }

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

    if (mImg.isNull())
    {
        ui->statusBar->showMessage(QString::number(size)  + " characters", 2000);
        return;
    }

    ui->statusBar->showMessage(QString::number(size) + " / " + QString::number(mNumWritableChars) + " characters", 2000);
}

void MainWindow::secretMsgTextEditPressHandler()
{
    on_actionEncode_triggered();
}

void MainWindow::threadEncodeHandler(bool succ)
{
    if (!succ) {

        if (mPointGenThread.isEncode()) statusBar()->showMessage(tr("Secret message is too big."), 2000);
        else statusBar()->showMessage(tr("Error while decoding image."), 2000);

    } else if (mPointGenThread.isEncode()) {
        QString saveFileName = QFileDialog::getSaveFileName(this, tr("Save Image"), mFileName, tr("Image Files (*.png *.bmp)"));

        if (!saveFileName.isEmpty()) {
            mPointGenThread.getImg().save(saveFileName);
            mViewStegoPixmap.convertFromImage(mPointGenThread.getImg());
            ui->ImageStegoView->setPixmap(mViewStegoPixmap.scaled(mScaleFactor * mViewStegoPixmap.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

            qDebug() << ui->scrollArea->horizontalScrollBar()->value();
            qDebug() << ui->scrollArea_2->horizontalScrollBar()->value();
            ui->scrollArea_2->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value());
            qDebug() << ui->scrollArea_2->horizontalScrollBar()->value();

            ui->scrollArea_2->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value());
        }
    }

    ui->encodeButton->setEnabled(true);
    ui->decodeButton->setEnabled(true);
    mPgBar->reset();
    mPgBar->setVisible(false);
}

void MainWindow::receiveSecretMsg(QByteArray msgBytes, bool compressed, bool encrypted)
{
    if (encrypted)
    {
        msgBytes = decrypt(msgBytes);
    }

    if (compressed)
    {
        msgBytes = qUncompress(msgBytes);
    }

    if (ui->radioButtonText->isChecked())
    {
        ui->secretMsgView->setPlainText(QString(msgBytes));
    }
    else
    {
        QString saveDataFileName = QFileDialog::getSaveFileName(this, tr("Save Image"), mFileName);

        if (!saveDataFileName.isEmpty()) {

            QFile dataFile(saveDataFileName);

            if (!dataFile.open(QIODevice::WriteOnly))
            {
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
    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open Image"), mFileName.isEmpty() ? QString() : mFileName, tr("Image Files (*.png *.bmp)"));

    if (!tmpFileName.isEmpty()) {

        openImage(tmpFileName);
        mScaleFactor = 1.0f;
    }
}

void MainWindow::openImage(QString name)
{
    mFileName = name;

    mImg.load(mFileName);
    mViewPixmap.convertFromImage(mImg);
    ui->ImageView->setPixmap(mViewPixmap);

    ui->decodeButton->setEnabled(true);

    ui->makeAnalysisButton->setEnabled(true);

    ui->compressSlider->setEnabled(true);

    ui->actionZoom_In_25->setEnabled(true);

    ui->actionZoom_Out_25->setEnabled(true);

    ui->action_Normal_Size->setEnabled(true);

    if (!mPassword.isEmpty()) ui->encryptCheckBox->setEnabled(true);

    ui->encodeMaxCheckBox->setEnabled(true);

    if (ui->radioButtonText->isChecked())
    {
        if (!ui->secretMsgView->toPlainText().isEmpty()) ui->encodeButton->setEnabled(true);
    }
    else
    {
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

void MainWindow::on_actionEncode_triggered()
{
    mPgBar->setMinimum(0);
    mPgBar->setVisible(true);

    ui->encodeButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);

    PointGenThread::Color color;
    if (ui->encodeMaxCheckBox->isChecked()) {
        color = PointGenThread::NONE;
    } else {
        if (ui->red_radio->isChecked()) {
            color = PointGenThread::RED;
        } else if (ui->green_radio->isChecked()) {
            color = PointGenThread::GREEN;
        } else if (ui->blue_radio->isChecked()){
            color = PointGenThread::BLUE;
        } else if (ui->all_radio->isChecked()){
            color = PointGenThread::ALL;
        } else {
            color = PointGenThread::NONE;
        }
    }

    mPointGenThread.setUp(mImg, mSecretBytes, qChecksum(mPassword.toStdString().c_str(), mPassword.size()), true,
                          (ui->compressSlider->value())? true : false,
                          ui->encryptCheckBox->isChecked(),
                          ui->encodeMaxCheckBox->isChecked(),
                          color);
    mPointGenThread.start();
}

void MainWindow::on_actionDecode_triggered()
{
    mPgBar->setMinimum(0);
    mPgBar->setVisible(true);

    ui->encodeButton->setEnabled(false);
    ui->decodeButton->setEnabled(false);

    QByteArray dummy;
    mPointGenThread.setUp(mImg, dummy, qChecksum(mPassword.toStdString().c_str(), mPassword.size()), false);
    mPointGenThread.start();
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
    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open Data"), mDataFileName.isEmpty() ? QString() : mDataFileName);

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

            ui->encodeMaxCheckBox->setEnabled(true);
            ui->red_radio->setEnabled(true);
            ui->green_radio->setEnabled(true);
            ui->blue_radio->setEnabled(true);
            ui->all_radio->setEnabled(true);
        }

        dataFile.close();
    }
}

