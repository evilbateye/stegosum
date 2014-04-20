#ifndef RASTER_H
#define RASTER_H

#include "stegosum.h"
#include "variation.h"
#include <iostream>

//! Modul zabezpečuje vkladanie tajnej správy do rastrových obrázkov.
/*!
Modul pre rastrovú grafiku dedí od modulu Stegosum a slúži na prácu s rastrovými
obrázkami, konkrétne obrázkami typu BMP a PNG. Modul pre rastrovú grafiku
obsahuje potrebné členské premenné pre uchovanie informácie o zvolených farbách
pixelov (do ktorých sa má kódovať tajná správa) a informácie potrebné pre dopredný
algoritmus vkladania.
*/
class Raster : public Stegosum
{
private:

    /*!
     * \brief Štruktúra, ktorá zbiera štatistické informácie o priebehu steganografickej funkcie.
     *
     *  Používa sa hlavne pri algoritme dopredného vkladania, ktorý je detailne opísaný v teoretickej časti diplomovej práce.
     *  Slúži na porovnanie efektívnosti algoritmu dopredného vkladania oproti bežnému LSB steganografickému algoritmu.
     */
    struct statsObj {
        //! Počet bitov zakódovaných do červenej farby.
        qint32 bitsR;
        //! Počet bitov zakódovaných do zelenej farby.
        qint32 bitsG;
        //! Počet bitov zakódovaných do modrej farby.
        qint32 bitsB;
        //! Počet bitov zakódovaných do všetkých farieb.
        qint32 total;
    };

    /*!
     * \brief Štruktúra predstavujúca permutáciu bez opakovania.
     *
     *  Využíva sa pri algoritme dopredného vkladania a slúži na výpočet vhodného poradia farebných oktetov (RGB),
     *  do ktorých sa má skryť tajná správa. Zvolené poradie sa vzťahuje na farebné oktety celého obrázka.
     */
    struct ColorPermutation {
        /*!
         * \brief Kód zvolenej permutácie.
         *
         *  Kód sa priamo pripája na začiatok tajnej správy ako súčasť metedát.
         *  Je implementovaný ako jednoduché poradie, v akom bola permutácia
         *  vygenerovaná pomocou funkcie next.
         */
        int code;
        /*!
         * \brief Pole troch čísel predstavujúce farebnú permutáciu.
         */
        int permutation[3];

        ColorPermutation() {
            code = 0;
            for (int i = 0; i < 3; i++) permutation[i] = i;
        }
        /*!
         * \brief Metóda pre vygenerovanie ďalšej permutácie.
         * \return Vracia true ak sa podarilo vygenerovať ďalšiu permutáciu, v opačnom prípade vracia false.
         */
        bool next() {
            code++;
            return std::next_permutation(permutation, permutation + 3);
        }
        /*!
         * \brief Nastavuje túto permutáciu na základe inej.
         * \param p Permutácia, na základe ktorej sa nastavuje táto permutácia.
         */
        void set(ColorPermutation & p) {
            code = p.code;
            for (int i = 0; i < 3; i++) permutation[i] = p.permutation[i];
        }
        /*!
         * \brief Nastavuje kód tejto permutácie.
         * \param code Kód, na základe ktorého sa má táto permutácia nastaviť.
         *
         *  Okrem nastavenia kódu funkcia nastaví aj pole čísel permutation tak,
         *  aby zodpovedalo danému kódu.
         */
        void setCode(int code) {
            reset();
            while (this->code != code) next();
        }
        /*!
         * \brief Funkcia resetuje permutáciu.
         *
         *  Nastavý kód a všetky prvky poľa permutation na hodnotu 0.
         */
        void reset() {
            code = 0;
            for (int i = 0; i < 3; i++) permutation[i] = i;
        }

        void print() {
            std::cout << "color permutation " << code << " :";
            for (int i = 0; i < 3; i++) std::cout << " " << permutation[i];
            std::cout << std::endl;
        }
    };

public:
    Raster(const QString & name);
    bool Encode();
    bool Decode();
    void saveStegoImg(QString &name);
    QPair<QImage, QImage> scaleImgs(float factor);
    void setSelectedImgs(Utils::DisplayImageColor color);
    QPair<QImage, QImage> getImgs(Utils::DisplayImageColor color) { return qMakePair(mSelectionIn[color], mSelectionOut[color]); }

private:
    /*!
     * \brief Pole štyroch prvkov statsObj.
     *
     *  Nultý prvok sa zameriava na oktety nesúce informáciu 00.
     *  Prvý prvok sa zameriava na oktety nesúce informáciu 01.
     *  Tretí prvok sa zameriava na oktety nesúce informáciu 10.
     *  Štvrtý prvok sa zameriava na oktety nesúce informáciu 11.
     */
    statsObj mStats[4];
    //! Dvojica obsahujúca typ modifikácie krycieho obrázka a modifikovaný rastrový krycí obrázok.
    QMap<Utils::DisplayImageColor, QImage> mSelectionIn;
    //! Dvojica obsahujúca typ modifikácie stegoobrázka a modifikovaný rastrový stegoobrázok.
    QMap<Utils::DisplayImageColor, QImage> mSelectionOut;

    /*!
     * \brief Modifikuje obrázok takým spôsobom, aby bola viditeľná LSB rovina.
     * \param image Stegoobrázok alebo krycí obrázok pripravený na modifikáciu.
     * \param color Farba LSB roviny, ktorú chceme zviditeľniť.
     */
    void convertToLSB(QImage & image, Utils::DisplayImageColor color);
    /*!
     * \brief Metóda konvetuje celé číslo na vektor bool hodnôt.
     * \param msgSize celé číslo
     * \param shift posunutie
     * \param msgBoolVect vektor pre uloženie výsledku
     */
    void numToBits(quint32 msgSize, quint32 shift, QVector<bool> & msgBoolVect);
    /*!
     * \brief Metóda inicializuje pseudonáhodný generátor čísel.
     * \param image obrázok použitý na inicializáciu, využívajú sa jeho prvé dva pixely
     * \param key kľúč použitý na inicializáciu, získava sa premenou hesla zadaného používateľom
     */
    void setSeed(QImage & image, quint16 key);
    /*!
     * \brief Metóda naplní vektor pixelov pixelmi daného obrázka.
     * \param pixVect vektor, ktorý chceme naplniť
     * \param image obrázok, ktorého pixelmi sa vektor napĺňa
     */
    void fillPixelVector(QVector<QRgb *> & pixVect, QImage & image);
    /*!
     * \brief Metóda konvertuje vektor bool hodnôt na celé číslo.
     * \param numBitsCount počet prvkov vektora, ktoré je nutné premeniť
     * \param msgBoolVect vektor obsahujúci bool hodnoty
     * \return celé číslo
     */
    quint32 bitsToNum(qint32 numBitsCount, QVector<bool> & msgBoolVect);
    /*!
     * \brief Metóda vyberie za pomoci pseudonáhodného generátora čísel ďalší vhodný pixel obrázka.
     * \param start hranica vektora s pixelmi pixVect
     * \param pixVect vektor s pixelmi obrázka
     * \return Vracia smerník na ďalší vhodný pixel.
     *
     *  Hranica start oddeľuje vybraté pixely vektora pixVect od nevybratých.
     *  Vybratým sa stáva pixel ak ho funkcia nextPixel vyberie na základe náhody a presunie do vrchných častí
     *  vektora pixVect k ostatným vybratým pixelom. Tým sa zabezpečí, že pole pixelov, z ktorých sa vyberá,
     *  obsahuje vždy iba doposiaľ nevybraté pixely a nedochádza ku kolíziám.
     */
    QRgb * nextPixel(qint32 &start, QVector<QRgb *> &pixVect);
    /*!
     * \brief Metóda nastavuje štatistickú štruktúru na základe príchodzích štatistík.
     * \param stats štatistická štruktúra, ktorá sa nastavuje
     * \param bits počet bitov zakódovaných do daného farebného oktetu
     * \param color farebný oktet, do ktorého sa kóduje tajná správa
     */
    void setStats(statsObj * stats, quint8 bits, quint8 color);
    /*!
     * \brief Metóda zresetuje pole obsahujúce štatistické štruktúry, t.j. nastavý štatistiku na 0.
     * \param stats pole obsahujúce štatistické štruktúry
     */
    void resetStats(statsObj *stats);
    /*!
     * \brief Vonjakšia časť algoritmu dopredného vkladania.
     * \param start hranica vektora s pixelmi pixVect
     * \param image krycí obrázok
     * \param key kľúč použitý na inicializáciu, získava sa premenou hesla zadaného používateľom
     * \param msgBVect vektor obsahujúci bity tajnej správy
     * \param pixVect vektor s pixelmi obrázka
     * \return Počet pixelov obrázka, do ktorých bola vložená tajná správa.
     *
     *  Táto časť algoritmu vyberá vhodné farebné permutácie a vhodné variácie
     *  bitov oktetov obrázka tak, aby bola tajná správa čo najkratšia.
     *  Vo vnútri funkcia volá ďalšie preťažené funkcie pre algoritmus dopredného vkladania.
     */
    qint32 encodeLookAhead(qint32 & start, QImage & image, quint16 key, QVector<bool> & msgBVect, QVector<QRgb *> & pixVect);
    /*!
     * \brief Vnútorná časť algoritmu dopredného vkladania.
     * \param start hranica vektora s pixelmi pixVect
     * \param variation zvolená variácia bitov oktetov obrázka
     * \param permutation zvolená permutácia farieb oktetov pre všetky pixely obrázka
     * \param msgBVect vektor obsahujúci bity tajnej správy
     * \param pixVect vektor s pixelmi obrázka
     * \return Počet pixelov obrázka, do ktorých bola vložená tajná správa.
     *
     *  Táto časť algoritmu dopredného vkladania využíva vedomosti o algoritme dopredného vkladania,
     *  ktoré sú opísané v teoretickej časti diplomovej práce a na základe neho vkladá bity tajnej
     *  správy do pixelov obrázka.
     */
    qint32 encodeLookAhead(qint32 & start, Variation & variation, ColorPermutation & permutation, QVector<bool> &msgBVect, QVector<QRgb *> & pixVect);
    /*!
     * \brief Metóda slúži na dekódovanie tajnej správy, ktorá bola zakódovaná pomocou algoritmu dopredného vkladania, z pixelov obrázka.
     * \param start hranica vektora s pixelmi pixVect
     * \param numOfBitsToDecode dĺžka tajnej správy v bitoch
     * \param msgBVect vektor, ktorý sa naplní bitmi tajnej správy
     * \param pixVect vektor s pixelmi obrázka
     * \return Počet pixelov obrázka, z ktorých bola vyextrahovaná tajná správa.
     */
    qint32 decodeLookAhead(qint32 & start, qint32 numOfBitsToDecode, QVector<bool> & msgBVect, QVector<QRgb *> & pixVect);
    /*!
     * \brief Metóda inicializuje pseudonáhodný generátor čísel a posunie sekvenciu generovaných čísel o požadovaný počet krokov.
     * \param image obrázok použitý na inicializáciu, využívajú sa jeho prvé dva pixely
     * \param key kľúč použitý na inicializáciu, získava sa premenou hesla zadaného používateľom
     * \param move počet krokov, o koľko sa má generátor čísel posunúť
     */
    void moveSequence(QImage & image, quint16 key, qint32 move);
    /*!
     * \brief Všeobecná funkcia pre vloženie bitov tajnej správy do obrázka.
     * \param start hranica vektora s pixelmi pixVect
     * \param pixVect vektor s pixelmi obrázka
     * \param toHowManyBits počet bitov farebného oktetu, do ktorých sa má vložiť tajná správa, začínajúc od LSB
     * \param colors typ oktetov podľa farby, do ktorých sa má vložiť tajná správa
     * \param numPars počet parametrov, metóda príjma variabilný počet parametrov vo forme dvojíc počet bitov a číselná hodnota
     * \return Vracia počet pixelov, ktoré bolo potrebné vygenerovať na vloženie tajnej správy.
     *
     *  Preťažená funkcia, ktorá sa využíva ako obal v prípade, ak chceme vložiť tajnú správu
     *  vo forme variabilného počtu celočíselných hodnôt.
     */
    qint32 encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::EncodeColorsObj colors, quint8 numPars, ...);
    /*!
     * \brief Všeobecná funkcia pre vloženie bitov tajnej správy do obrázka.
     * \param start hranica vektora s pixelmi pixVect
     * \param pixVect vektor s pixelmi obrázka
     * \param toHowManyBits počet bitov farebného oktetu, do ktorých sa má vložiť tajná správa, začínajúc od LSB
     * \param colors typ oktetov podľa farby, do ktorých sa má vložiť tajná správa
     * \param vector vektor obsahujúci bity tajnej správy prichystané na zakódovanie do obrázka
     * \return Vracia počet pixelov, ktoré bolo potrebné vygenerovať na vloženie tajnej správy
     *
     *  Funkcia sa využíva v prípade ak máme k dispozícii vektor s pixelmi obrázka.
     *  Funkcia si sama odoberie potrebný počet pixelov na zakódovanie správy.
     */
    qint32 encodeToPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::EncodeColorsObj colors, QVector<bool> & vector);
    /*!
     * \brief Všeobecná funkcia pre vloženie bitov tajnej správy do obrázka.
     * \param pixel pixel obrázka do ktorého sa majú vložiť bity tajnej správy
     * \param toHowManyBits počet bitov farebného oktetu, do ktorých sa má vložiť tajná správa, začínajúc od LSB
     * \param colors typ oktetov podľa farby, do ktorých sa má vložiť tajná správa
     * \param vector vektor obsahujúci bity tajnej správy prichystané na zakódovanie do obrázka
     *
     *  Funkcia vkladá tajnú správu iba do daného jedného pixelu obrázka.
     */
    void encodeToPixel(QRgb * pixel, quint8 toHowManyBits, Utils::EncodeColorsObj colors, QVector<bool> & vector);
    /*!
     * \brief Všeobecná funkcia na dekódovanie bitov tajnej správy z obrázka.
     * \param start hranica vektora s pixelmi pixVect
     * \param pixVect vektor s pixelmi obrázka
     * \param toHowManyBits počet bitov farebného oktetu, z ktorých sa má vyextrahovať tajná správa, začínajúc od LSB
     * \param colors typ oktetov podľa farby, z ktorých sa má vyextrahovať tajná správa
     * \param numPars počet parametrov, metóda príjma variabilný počet parametrov vo forme dvojíc počet bitov a premenná na uloženie číselnej hodnoty
     * \return Vracia počet pixelov, ktoré bolo potrebné vygenerovať na dekódovanie tajnej správy.
     *
     *  Preťažená funkcia, ktorá sa využíva ako obal v prípade, ak chceme vyextrahovať tajnú správu
     *  do variabilného počtu premenných.
     */
    qint32 decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::EncodeColorsObj colors, quint8 numPars, ...);
    /*!
     * \brief Všeobecná funkcia na dekódovanie bitov tajnej správy z obrázka.
     * \param start hranica vektora s pixelmi pixVect
     * \param pixVect vektor s pixelmi obrázka
     * \param toHowManyBits počet bitov farebného oktetu, z ktorých sa má vyextrahovať tajná správa, začínajúc od LSB
     * \param colors typ oktetov podľa farby, z ktorých sa má vyextrahovať tajná správa
     * \param bitsSum počet bitov tajnej správy, ktoré chceme vyextrahovať
     * \param vector vektor, ktorý sa naplní vyextrahovanými bitmi tajnej správy
     * \return Vracia počet pixelov, ktoré bolo potrebné vygenerovať na dekódovanie tajnej správy.
     */
    qint32 decodeFromPixel(qint32 & start, QVector<QRgb *> & pixVect, quint8 toHowManyBits, Utils::EncodeColorsObj colors, qint32 bitsSum, QVector<bool> & vector);
};

#endif // RASTER_H
