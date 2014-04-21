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

/*!
 * \brief Abstraktná trieda, od ktorej dedia moduly kódujúce tajnú správu do rôznych krycích médií.
 *
 *  Modul Stegosum je daný abstraktnou triedou, ktorá presne definuje potrebné podmienky,
 *  ktoré musia spĺňať všetky ďalšie moduly, ktoré od neho dedia. Tie budúpribúdať
 *  s narastajúcou podporou pre rôzne druhy krycích obrázkov. Modul Stegosum
 *  ďalej obsahuje potrebné členské premenné pre uchovanie príznaku kódovania,
 *  kompresie, šifrovania a sprievodné informácie potrebné pre vizuálnu steganalýzu
 *  (vizuálna steganalýza je prepojená s triedou Stegosum, ktorá poskytuje informácie o
 *  pozmenenom stegoobrázku).
 */
class Stegosum : public QThread
{
    Q_OBJECT
public:
    explicit Stegosum(QWidget *parent = 0);
    /*!
     * \brief Metóda spúšťa vlákno vykonávajúce steganografiu.
     *
     *  Metóda sa rozhodne na základe príznaku mEncode a následne
     *  spustí vo vlákne kódovaciu alebo dekódovaciu funkciu.
     */
    void run();
    /*!
     * \brief Kódovacia funkcia.
     * \return Vracia true alebo false podľa úspešnosti funkcie.
     *
     *  Slúži na zakódovanie tajnej správy do krycieho média.
     */
    virtual bool Encode() = 0;
    /*!
     * \brief Dekódovacia funkcia.
     * \return Vracia true alebo false podľa úspešnosti funkcie.
     *
     *  Slúži na dekódovanie tajnej správy z krycieho média.
     */
    virtual bool Decode() = 0;
    /*!
     * \brief Nastavuje potrebné členské premenné pre objekt tiedy Stegosum.
     * \param mw Smerník na triedu, ktorá objekt vytvorila.
     * \param encode Príznak pre kódovanie alebo dekódovanie.
     * \param isCompress Príznak pre kompresiu.
     * \param isEncrypt Príznak pre šifrovanie.
     * \param isLookAhead Príznak pre algoritmus dopredného vkladania.
     * \param fppos Posunutie desatinnej čiarky.
     * \param isfpposmax Príznak pre výpočet maximálneho posunutia desatinnej čiarky.
     *
     */
    void setUp(MainWindow * mw, bool encode = true, bool isCompress = false, bool isEncrypt = false, bool isLookAhead = false, int fppos = 8, bool isfpposmax = false);
    //virtual QImage convertToLSB() = 0;
    /*!
     * \brief Ukladá modifikovaný stegoobrázok na špecifikované miesto na disku.
     * \param name miesto kam sa má obrázok uložiť
     */
    virtual void saveStegoImg(QString & name) = 0;
    /*!
     * \brief Zväčší alebo zmenší stegoobrázok a krycí obrázok.
     * \param factor zväčšovací (zmenšovací) faktor
     * \return Pár stegoobrázok a krycí obrázok po modifikácii.
     */
    virtual QPair<QImage, QImage> scaleImgs(float factor) = 0;        
    /*!
     * \brief Nastavuje typ obrázka na základe premennej mSelColor.
     * \param color oznamuje spôsob, akým sa majú stegoobrázok a krycí obrázok modifikovať pre vizuálnu steganalýzu
     *
     *  Pri zmene typu obrázka sa používateľovy ukážu iné konvertované verzie krycieho
     *  obrázka a stegoobrázka. V prípade ak dané verzie obrázkou neexistujú, tak sa vytvoria a uložia.
     */
    virtual void setSelectedImgs(Utils::DisplayImageColor color) = 0;
    /*!
     * \brief Vracia dvojicu stegoobrázok a krycí obrázok.
     * \param color vyberá požadovanú dvojicu modifikovaných obrázkov
     * \return Pár krycí obrázok a stegoobrázok.
     */
    virtual QPair<QImage, QImage> getImgs(Utils::DisplayImageColor color = Utils::COLOR_NONE) = 0;
    /*!
     * \brief Vracia informáciu o tom, či objekt triedy Stegosum je typu Raster alebo Vector.
     * \return Vracia true ak sa jedná o Raster, v opačnom prípade false.
     */
    bool isRaster() { return mIsRaster; }
    /*!
     * \brief Vracia informáciu o tom, či objekt triedy stegosum je nastavený na kódovanie alebo dekódovanie tajnej správy.
     * \return Vracia true ak sa jedná o kódovanie, v opačnom prípade false.
     */
    inline bool isEncode() { return mEncode; }    

signals:
    void updateProgress(int value);
    void setMaximum(int value);
    /*!
     * \brief Signál, ktorý sa spustí po tom, čo vlákno s kódovacou alebo dekódovacou funkciou ukončí svoju činnosť.
     * \param succ True ak kódovanie alebo dekódovanie prebehlo úspešne, false v opačnom prípade.
     */
    void succes(bool succ);
    /*!
     * \brief Signál, ktorý sa spustí po tom, čo vlákno s dekódovacou funkciou ukončí svoju činnosť.
     * \param message dekódovaná tajná správa
     * \param compressed dekódovaný príznak kompresie
     * \param encrypted dekódovaný príznak šifrovania
     */
    void sendMessage(QByteArray message, bool compressed, bool encrypted);
    /*!
     * \brief Signál, ktorý sa spustí, ak je nutné vypísať do konzoly oznam alebo chhybové hlásenie.
     * \param string oznam alebo chybové hlásenie
     */
    void writeToConsole(QString string);
    /*!
     * \brief Signál, ktorý sa spustí, ak je nutné vypísať do status baru oznam alebo chhybové hlásenie.
     * \param string oznam alebo chybové hlásenie
     */
    void writeToStatus(QString string);

public slots:

protected:
    bool mIsDebug;
    //! Referencia na tajnú správu vo forme poľa bajtov.
    QByteArray mMsg;
    //! Jedinečné číslo pre inicializáciu pseudonáhodného generátora čisel získané z hesla zadaného používateľom.
    quint16 mKey;
    //! Referencia na heslo, ktoré vloží používateľ do daného textového poľa v hlavnom okne.
    QString mPassword;
    //! Referencia na inštanciu triedy EncodeColorsObject.
    Utils::EncodeColorsObj mColors;
    //! Príznak mEncode rozhoduje, či sa v novom vlákne spustí funkcia pre zakódovanie alebo dekódovanie tajnej správy.
    bool mEncode;
    //! Je príznak, ktorý hovorí o tom, či bola tajná správa komprimovaná.
    bool mIsCompress;
    //! Je príznak, ktorý hovorí o tom, či bola tajná správa šifrovaná.
    bool mIsEncrypt;
    //! Je príznak, ktorý rozhoduje o použití bežného LSB algoritmu alebo algoritmu dopredného vkladania.
    bool mIsLookAhead;
    //! Posun desatinnej čiarky pre vektorový krycí obrázok.
    int mFPPos;
    //! Je príznak pre výpočet maximálneho posunu desatinnej čiarky.
    bool mIsFPPosMax;
    //! Je príznak, ktorý oznamuje či je objekt triedy Stegosum typu Raster alebo Vektor.
    bool mIsRaster;
    //! Aktuálne vybratá modifikácia stegoobrázka a krycieho obrázka pre vizuálnu steganalýzu.
    Utils::DisplayImageColor mSelColor;
};

#endif // POINTGENTHREAD_H
