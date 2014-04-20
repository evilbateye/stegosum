#ifndef UTILS_HPP
#define UTILS_HPP

#include <QtCore>

#define SUFFLEEOFFSET 2
#define NUM_OF_SIZE_BITS 24
#define NUM_OF_SETTINGS_BITS 6
#define NUM_OF_SETTINGS_PIXELS ((NUM_OF_SETTINGS_BITS % 3 == 0) ? (NUM_OF_SETTINGS_BITS / 3) : ((NUM_OF_SETTINGS_BITS / 3) + 1))
#define NUM_OF_VARIATIONS_BITS 8
#define NUM_OF_PERMUTATIONS_BITS 3

//! Pomocná trieda obsahujúca konštanty a statické funkcie.
/*!
Utils obsahuje statické konštanty a pomocné funkcie, ktoré
sú potrebné v rôznych častiach aplikácie.
*/
class Utils
{
public:
    //! Enum obsahujúci typy modifikácií nad stegoobrázkom a krycím obrázkom v rámci vizuálnej steganalýzy.
    enum DisplayImageColor {COLOR_RED = 1, COLOR_GREEN = 2, COLOR_BLUE = 4, COLOR_ALL = 7, COLOR_NONE = 0, COLOR_PREV = -1, COLOR_ILUM = -2};
    struct EncodeColorsObj {
        bool rgb[3];
        quint8 numOfselected;

        EncodeColorsObj(bool r, bool g, bool b) { set(r, g, b); }
        EncodeColorsObj() { set(false, false, false); }

        void set(bool r, bool g, bool b) {
            quint8 c = 0;
            if ((rgb[0] = r)) c++;
            if ((rgb[1] = g)) c++;
            if ((rgb[2] = b)) c++;
            numOfselected = c;
        }
    };
    Utils();
    static qint32 pixelsNeeded(qint32 bits, quint8 selectedColors, quint8 toHowManyBits = 1);
    static QByteArray encrypt(const QByteArray &msg, const QString &password);
    static QByteArray decrypt(const QByteArray & msg, const QString & password);

};

#endif // UTILS_HPP
