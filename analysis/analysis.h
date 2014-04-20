#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <QImage>

//! Abstraktná trieda, od ktorej dedia všetky moduly zaoberajúce sa steganalýzou.
class Analysis
{
public:
    //! Steganalýza sa vykonáva vzhľadom na jednotlivé farebné zložky rastrového obrázka.
    enum Color {
        ANALYSIS_COLOR_RED,
        ANALYSIS_COLOR_GREEN,
        ANALYSIS_COLOR_BLUE
    };

    Analysis();
    //! Predstavuje začiatok výpočtu štatistickej funkcie.
    /*!
    \param image analyzovaný stegoobrázok
    \param color farebná zložka, ktorá sa má analyzovať
    \param overlap príznak špecifický pre RS analýzu

    Je to rozhranie, ktoré sú povinné implementovať všetky triedy dediace od triedy Analysis.
    */
    virtual void analyse(const QImage & image, Color color, bool overlap) = 0;
    //! Metóda získa požadovaný farebný oktet pixela.
    /*!
    \param druh farebného oktetu, ktorý chceme získať
    \param pixel pixel obrázka, z ktorého chceme vyextrahovať oktet
    \return Požadovaný farebný oktet pre daný pixel obrázka.
    */
    quint8 getColor(Color color, const QRgb *pixel);
    //! Metóda vracia odhadovanú dĺžku tajnej správy v médiu.
    /*!
    \return Odhadovaná dĺžka tajnej správy v médiu.
    */
    double getMessageLength() { return mMessageLength; }

protected:
    //! Predstavuje odhadovanú dĺžku tajnej správy v skúmanom médiu.
    double mMessageLength;
};

#endif // ANALYSIS_H
