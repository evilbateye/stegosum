#ifndef UTILS_HPP
#define UTILS_HPP

#include <QtCore>

/*!
 * \brief Pomocná trieda obsahujúca konštanty a statické funkcie.
 *
 *  Utils obsahuje statické konštanty a pomocné funkcie, ktoré
 *  sú potrebné v rôznych častiach aplikácie.
 */
class Utils
{
public:
    /*!
     * \brief Posunutie označujúce počet pixelov od začiatku obrázka pri rastrovej steganografii.
     *
     *  Prvé dva pixely rastrového obrázka sa využívaju na inicializáciu
     *  pseudonáhodného generátora čísel (spolu s heslom, ktoré zadáva používateľ).
     *  Do týchto pixelov sa nevkladajú bity tajnej správy.
     */
    static const int SUFFLEEOFFSET = 2;
    //! Počet bitov, ktoré zaberá dĺžka tajnej správy pri rastrovej steganografii.
    static const int NUM_OF_SIZE_BITS = 24;
    //! Počet bitov, ktoré zaberajú nastavenia (šifrovanie, kompresia, výber farebných oktetov, ...) pri rastrovej steganografii.
    static const int NUM_OF_SETTINGS_BITS = 6;
    //! Počet pixelov, ktoré zaberajú nastavenia pri rastrovej steganografii.
    static const int NUM_OF_SETTINGS_PIXELS = ((Utils::NUM_OF_SETTINGS_BITS % 3 == 0) ? (Utils::NUM_OF_SETTINGS_BITS / 3) : ((Utils::NUM_OF_SETTINGS_BITS / 3) + 1));
    //! Počet bitov, ktoré sú potrebné na zakódovanie zvolenej variácie bitov pre všetky zvolené farebné oktety obrázka pri algoritme dopredného vkladania.
    static const int NUM_OF_VARIATIONS_BITS = 8;
    //! Počet bitov, ktoré sú potrebné na zakódovanie zvolenej permutácie farieb farebných oktetov pre všetky pixely obrázka pri algoritme dopredného vkladania.
    static const int NUM_OF_PERMUTATIONS_BITS = 3;
    //! Počet bitov, ktoré sa odoberú pri premene tajnej správy na dekadický reťazec číslic pri vektorovej steganografii.
    static const int BIT_ENCODING = 63;
    //! Maximálna dekadická hodnota, ktorá má rovnaký počet číslic, ako maximálne číslo zakódované na BIT_ENCODING bitoch.
    static const unsigned long long BE_MAX_DEC_NUM = 9999999999999999999ull;
    //! Počet cifier maximálnej dekadickej hodnoty BE_MAX_DEC_NUM.
    static const int BE_CIPHERS_COUNT = 19;
    //! Počet cifier, ktoré sa využijú na zakódovanie dĺžky tajnej správy pri vektorovej steganografii.
    static const int NUM_MSG_LEN_NUMS = 8;
    //! Povolená odchýlka uhla pri rozhodovaní sa o rovnobežnosti dvoch úsečiek pri vektorovej steganografii.
    static const int ANGLE_PREC = 1;
    //! Veľkosť bodov, ktoré sa zobrazia počas vizuálnej steganalýzy nad vektorovými obrázkami.
    static const int SHOW_POINTS_W_AND_H = 5;

    //! Enum obsahujúci typy modifikácií nad stegoobrázkom a krycím obrázkom v rámci vizuálnej steganalýzy.
    enum DisplayImageColor {COLOR_RED = 1, COLOR_GREEN = 2, COLOR_BLUE = 4, COLOR_ALL = 7, COLOR_NONE = 0, COLOR_PREV = -1, COLOR_ILUM = -2};
    //! Štruktúra pre výber farebných oktetov v rámci rastrovej steganografie, do ktorých sa bude vkladať tajná správa.
    struct EncodeColorsObj {
        //! Pole troch bool hodnôt pre označenie výberu farebných oktetov.
        bool rgb[3];
        //! Počet farieb, ktoré boli vybraté a do ktorých sa bude kódovať tajná správa.
        quint8 numOfselected;
        /*!
         * \brief Konštruktor nastavujúci zvolené farebné oktety.
         * \param r true ak sa má tajná správa kódovať do červených oktetov, v opačnom prípade false
         * \param g true ak sa má tajná správa kódovať do zelených oktetov, v opačnom prípade false
         * \param b true ak sa má tajná správa kódovať do modrých oktetov, v opačnom prípade false
         *
         *  Celkový počet zvolených farieb sa automaticky dopočíta.
         */
        EncodeColorsObj(bool r, bool g, bool b) { set(r, g, b); }
        /*!
         * \brief Bezparametrický konštruktor pre nastavenie zvolených farebných oktetov.
         *
         *  Všetky farebné oktety sú označené ako false a počet zvolených farieb je rovný 0.
         */
        EncodeColorsObj() { set(false, false, false); }
        /*!
         * \brief Metóda pre nastavenie zvolených farebných oktetov.
         * \param r true ak sa má tajná správa kódovať do červených oktetov, v opačnom prípade false
         * \param g true ak sa má tajná správa kódovať do zelených oktetov, v opačnom prípade false
         * \param b true ak sa má tajná správa kódovať do modrých oktetov, v opačnom prípade false
         *
         *  Metóda sa využíva v konštruktoroch triedy EncodeColorsObj.
         */
        void set(bool r, bool g, bool b) {
            quint8 c = 0;
            if ((rgb[0] = r)) c++;
            if ((rgb[1] = g)) c++;
            if ((rgb[2] = b)) c++;
            numOfselected = c;
        }
    };
    Utils();
    /*!
     * \brief Metóda pre výpočet potrebného počtu pixelov na zakódovanie bitov tajnej správy.
     * \param bits počet bitov tajnej správy
     * \param selectedColors počet farieb, ktoré boli vybraté a do ktorých sa bude kódovať tajná správa.
     * \param toHowManyBits počet bitov farebného oktetu, z ktorých sa má vyextrahovať tajná správa, začínajúc od LSB
     * \return Vracia potrebný počet pixelov na zakódovanie bitov tajnej správy.
     */
    static qint32 pixelsNeeded(qint32 bits, quint8 selectedColors, quint8 toHowManyBits = 1);
    /*!
     * \brief Metóda, ktorá slúži na zašifrovanie tajnej správy.
     * \param msg pole bajtov obsahujúce tajnú správu
     * \param password heslo pre zašifrovanie tajnej správy
     * \return zašifrovaná tajná správa
     */
    static QByteArray encrypt(const QByteArray &msg, const QString &password);
    /*!
     * \brief Metóda, ktorá slúži na dešifrovanie tajnej správy.
     * \param msg pole bajtov obsahujúce zašifrovanú tajnú správu
     * \param password heslo pre dešifrovanie tajnej správy
     * \return dešifrovaná tajná správa
     */
    static QByteArray decrypt(const QByteArray & msg, const QString & password);

};

#endif // UTILS_HPP
