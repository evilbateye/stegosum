#ifndef VECTOR_H
#define VECTOR_H

#include "stegosum.h"
#include <QDomDocument>
#include <cmath>

/*!
 * \brief Modul zabezpečuje vkladanie tajnej správy do vektorových obrázkov.
 *
 *  Modul pre vektorovú grafiku sa špecifikuje na prácu s vektorovými obrázkami, konkrétne
 *  na obrázky typu svg. Modul pre vektorovú grafiku obsahuje potrebné členské
 *  premenné pre uchovanie informácií o posunutí desatinnej čiarky pri kódovaní tajnej
 *  správy a príznak pre výpočet maximálneho povoleného posunutia desatinnej čiarky.
 */
class Vector : public Stegosum
{
public:
    Vector(const QString & name);
    ~Vector();
    bool Encode();
    bool Decode();
    void saveStegoImg(QString &name);
    QPair<QImage, QImage> scaleImgs(float factor);
    void setSelectedImgs(Utils::DisplayImageColor color);
    QPair<QImage, QImage> getImgs(Utils::DisplayImageColor color);
    /*!
     * \brief Metóda premieňa desatinné číslo na reťazec znakov bez desatinnej čiarky.
     * \param number desatinné číslo
     * \param fppos pozícia desatinnej čiarky v desatinnom čísle
     * \return reťazec znakov
     */
    static QString digitStream(qreal number, int fppos);
    /*!
     * \brief Metóda premieňa reťazec znakov na desatinné číslo.
     * \param digitStream reťazec znakov
     * \param ffpos pozícia desatinnej čiarky v novom desatinnom čísle
     * \return nové desatinné číslo
     */
    static qreal streamToReal(QString digitStream, int ffpos);

private:
    //! Dvojica obsahujúca typ modifikácie krycieho obrázka a modifikovaný vektorový krycí obrázok.
    QMap<Utils::DisplayImageColor, QByteArray> mSelXmlIn;
    //! Dvojica obsahujúca typ modifikácie stegoobrázka a modifikovaný vektorový stegoobrázok.
    QMap<Utils::DisplayImageColor, QByteArray> mSelXmlOut;
    //! Veľkosť krycieho obrázka a stegoobrázka, potrebná pri zmene mierky.
    QSize mSize;

    /*!
     * \brief Metóda príjma vektorový krycí obrázok alebo stegoobrázok a zvýrazňuje v ňom body podporovaných polygónov.
     * \param arr krýcí obrázok alebo stegoobrázok, v podobe xml SVG súboru
     */
    void iluminatePoints(QByteArray & arr);
    /*!
     * \brief Metóda vypočíta celkovú dĺžku tajnej správy.
     * \param msg tajná správa vo forme reťazca znakov
     * \param fpppos posunutie desatinnej čiarky
     * \return celková dĺžka tajnej správy vzhľadom na posunutie desatinnej čiarky
     */
    qreal totalMessageLength(QString msg, int fpppos);
    /*!
     * \brief Dĺžka všetkých čiar podporovaného polygónu.
     * \param in pole súradníc podporovaného polygónu
     * \return dĺžka všetkých čiar
     */
    qreal polyLineLength(QStringList & in);
    /*!
     * \brief Metóda na dekódovanie ďalšieho bodu s obsahom tajnej správy.
     * \param nodes list všetkých polygónov v xml dokumente
     * \param msg referencia na reťazec znakov, sem sa vkladá tajná správa vyextrahovaná z bodu vo forme reťazca číslic
     * \param polylineStart z ktorého polygónu v poradí sa extrahuje súčasná správa
     * \param lineStart z ktorého bodu pre daný polygón sa extrahuje súčasná správa
     * \param c prenesenie zvyšku tajnej správy medzi rôznymi polygónmi, časť správy môže ostať zakódovaná v predchádzajúcom
     * \param fppos posunutie desatinnej čiarky potrebné pri premene tajnej správy na reťazec číslic
     * \return Vracia true ak bola tajná správa z bodu úspešne vyextrahovaná, v opačnom prípade false.
     */
    bool nextPointSecret(QDomNodeList & nodes, QString &msg, int & polylineStart, int &lineStart, qreal & c, int fppos);
    /*!
     * \brief Metóda inkrementuje alebo dekrementuje poslednú cifru desatinného čísla.
     * \param real desatinné číslo, ktoré chceme pozmeniť
     * \param inc hodnota zmeny, záporná hodnota dekrementuje
     * \param fppos posunutie desatinnej čiarky
     * \return Vracia desatinné číslo po zmene vo forme reťazca cifier.
     */
    QString addToReal(qreal real, int inc, int fppos);
    /*!
     * \brief Metóda pripravuje polygón na vloženie tajnej správy.
     * \param in zoznam všetkých bodov polygónu
     *
     *  Pred vložením tajnej správy môžu existovať v polygóne body,
     *  ktoré ležia na čiarách polygónu. Tieto body je nutné vymazať,
     *  pretože inak by ich dekódovacia metóda považovala za body obsahujúce tajnú správu.
     */
    void preparePolyLine(QStringList & in);
    /*!
     * \brief Metóda príjma dve čiary a zisťuje, čí sú rovnobežné.
     * \param line prvá čiara
     * \param nextLine druhá čiara
     * \return Vracia false ak sú čiary rovnobežné, inak vracia true.
     */
    bool linesNotParallel(QLineF & line, QLineF & nextLine);
    /*!
     * \brief Metóda nastavuje cifru čísla na vybranej pozícii na zvolenú hodnotu.
     * \param number číslo, ktorého cifru chceme zmeniť
     * \param digit hodnota cifry, ktorú chceme do čísla vložiť
     * \param pos pozícia v čísle, kam chceme cifru vložiť začínajúc od najpravejšej cifry
     * \return Vracia pozmenené číslo vo forme reťazca číslic
     */
    QString setDigitAt(QString number, int digit, int pos = 0);
    /*!
     * \brief Metóda vracia cifru na predvolenej pozícii daného čísla.
     * \param number číslo, ktorého cifru chceme vyextrahovať
     * \param pos pozícia v čísle, odkiaľ chceme cifru vyextrahovať začínajúc od najpravejšej cifry
     * \return Vracia cifru na predvolenej pozícii daného čísla.
     */
    int digitAt(QString number, int pos = 0);
    /*!
     * \brief Metóda zisťuje, či je daný polygón podporovaný a v prípade ak áno, vyextrahuje jeho súradnice pre ostatné metódy.
     * \param path súradnice polygónu v podobe v akej sa nachádzajú v xml dokumente
     * \param list výstupný zoznam súradníc polygónu v upravenej podobe
     * \param z príznak, ktorý oznamuje, či sa jedná o uzatvorený alebo otvorený polygón
     * \return Vracia true, ak je daný polygón podporovaný, inak vracia false.
     */
    bool isLineSupported(QString path, QStringList &list, QChar &z);
    /*!
     * \brief Metóda koriguje presnosť v prípade, ak odvesny daného bodu nevracajú preponu s požadovanou dĺžkou.
     * \param precise požadovaná dĺžka prepony
     * \param A dĺžka prvej odvesny
     * \param B dĺžka druhej odvesny
     * \param fppos posunutie desatinnej čiarky
     * \return Vracia true, ak sa podarilo modifikovať odvesny takým spôsobom, aby mala prepona požadovanú hodnotu, inak vracia false.
     */
    bool precisionCorrection(qreal precise, QString & A, QString & B, int fppos = -1);
    /*!
     * \brief Metóda vracia rozdiel medzi požadovanou dĺžkou prepony a dĺžkou získanou z odvesien a, b.
     * \param precise požadovaná dĺžka prepony
     * \param a dĺžka prvej odvesny
     * \param b dĺžka druhej odvesny
     * \param fppos posunutie desatinnej čiarky
     * \return Vracia rozdiel medzi požadovanou dĺžkou prepony a dĺžkou získanou z odvesien a, b.
     */
    int computeDifference(qreal precise, qreal a, qreal b, int fppos = -1);
    /*!
     * \brief Metóda premieňa tajnú správu vo forme poľa bajtov na reťazec desiatkových číslic.
     * \param arr reťazec číslic, ktorý sa naplní pozmenenou správou
     * \return Vracia počet číslic, ktoré boli vytvorené.
     */
    int encodeMessage(QString &arr);
    /*!
     * \brief Metóda príjma číslo, randomizuje ho a uloží ho ako reťazec desiatkových číslic.
     * \param enc číslo pripravené na randomizáciu
     * \param arr výstupný reťazec desiatkových číslic
     * \param ciphersC počet cifier, ktoré má obsahovať výsledné číslo
     * \param maxDec maxímálne desiatkové číslo s počtom číslic ciphersC
     */
    void randomizeWord(quint64 enc, QString &arr, int ciphersC = Utils::BE_CIPHERS_COUNT, quint64 maxDec = Utils::BE_MAX_DEC_NUM);
    /*!
     * \brief Metóda premieňa tajnú správu vo forme reťazca desiatkových číslic na pole bajtov.
     * \param res výstupné pole bajtov s premenenou tajnou správou
     * \param msg vstupný reťazec desiatkových číslic
     */
    void decodeMessage(QByteArray & res, QString msg);
    /*!
     * \brief Metóda príjma číslo, derandomizuje ho a jeho bity uloží do vektora bool hodnôt.
     * \param v výstupný vektor s bitmi čísla
     * \param w vstupné číslo
     * \param take počet bitov, ktoré zo vstupného čísla vyextrahovať
     * \param maxDec maxímálne desiatkové číslo s rovnakým počtom číslic ako vstupné číslo
     * \param bits počet bitov, ktorý sa využíval pri kódovaní tajnej správy na reťazec číslic
     */
    void derandomizeWord(QVector<bool> & v, quint64 w, int take = 0, quint64 maxDec = Utils::BE_MAX_DEC_NUM, int bits = Utils::BIT_ENCODING);
    /*!
     * \brief Metóda príjma počet bitov, a vracia počet cifier maximálneho desiatkového čísla s daným počtom bitov.
     * \param bits počet bitov
     * \return Vracia počet cifier.
     */
    int numberOfCiphers(int bits) { return  floor(log10(((1ull << bits) - 1))) + 1; }
    /*!
     * \brief Metóda pre výpočet mocniny vysokých čísel.
     * \param base základ mocniny
     * \param exp exponent
     * \return Vracia požadovanú mocninu.
     */
    quint64 power(int base, int exp) {
        quint64 r = 1;
        for (int i = 0; i < exp; i++) r *= base;
        return r;
    }
    /*!
     * \brief Metóda príjma počet cifier a vracia maximálne dekadické číslo s daným počtom cifier.
     * \param ciphC počet cifier
     * \return Vracia maximálne dekadické číslo s daným počtom cifier.
     */
    quint64 maximumDecadicNumber(int ciphC) { return power(10, ciphC) - 1; }
    /*!
     * \brief Metóda nuluje vektor bitov z ľavej strany až po prvý výskyt hodnoty 1 (true).
     * \param v vstupný vektor bitov
     * \return Vracia počet zmazaných núl (false).
     */
    int removeZeros(QVector<bool> & v);
    /*!
     * \brief Metóda príjma desiatkové číslo a zisťuje minimálny počet bitov potrebných na jeho zakódovanie.
     * \param n vstupné číslo
     * \return minimálny počet bitov potrebných na jeho zakódovanie
     */
    int minimumBitsCount(quint64 n) {
        int bits = 1;
        for (int i = 1; i < 64; i++) {
            if ((n >> i) & 1) bits = i + 1;
        }
        return bits;
    }

    int getFloatingPointPosition(qreal fp);
};

#endif // VECTOR_H
