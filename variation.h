#ifndef VARIATION_H
#define VARIATION_H

/*!
 * \brief Trieda predstavujúca variácie s opakovaním.
 *
 *  Využíva sa pri algoritme dopredného vkladania a slúži na výpočet vhodného
 *  poradia bitov vo farebnom oktete, do ktorých sa má vložiť tajná správa.
 *  Zvolené poradie sa vzťahuje na farebné oktety celého obrázka.
 */
class Variation
{
private:
    //! Počet prvkov, z množiný ktorých sa generujú variácie.
    int mN;
    //! Stupeň variácie.
    int mK;
    /*!
     * \brief Kód zvolenej variácie.
     *
     *  Kód sa priamo pripája na začiatok tajnej správy ako súčasť metedát.
     *  Je implementovaný ako jednoduché poradie, v akom bola variácia
     *  vygenerovaná pomocou funkcie next.
     */
    int mCode;
    //! Pole mK čísel predstavujúce variáciu bitov.
    int * mVariation;
    //! Celkový počet všetkých variácií pre daný počet prvkov mN a daný stupeň variácie mK.
    int mTotalVariations;
public:
    //! Konštruktor inicializuje objekt variácie z n prvkov a k-tej triedy. Kód má platnú hodnotu 0.
    Variation(int n, int k);
    //! Konštruktor inicializuje objekt variácie z objektu inej variácie v.
    Variation(Variation & v);
    //! Konštruktor inicializuje objekt prázdnej variácie. Kód variácie má neplatnú hodnotu -1.
    Variation();
    //! Metóda nastavuje túto variáciu na základe inej variácie v.
    void set(Variation & v);
    //! Metóda vracia počet prvkov mN, z ktorých sa tvoria variácie.
    int getN() { return mN; }
    //! Metóda vracia stupeň variácie mK.
    int getK() { return mK; }
    //! Metóda vracia celkový počet všetkých variácií mTotalVariations.
    int getTotalVariations() { return mTotalVariations; }
    /*!
     * \brief Metóda pre vygenerovanie ďalšej variácie.
     * \return Vracia true ak sa podarilo vygenerovať ďalšiu variáciu, v opačnom prípade vracia false.
     */
    bool next();
    //! Metóda vracia kód variácie mCode.
    int getCode() { return mCode; }
    //! Metóda vracia smerník na pole čísel predstavujúce variáciu mVariation.
    int * getVariation() { return mVariation; }
    /*!
     * \brief Nastavuje kód tejto variácie mCode.
     * \param code Kód, na základe ktorého sa má táto variácia nastaviť.
     *
     *  Okrem nastavenia kódu funkcia nastaví aj pole čísel mVariation tak,
     *  aby zodpovedalo danému kódu.
     */
    void setCode(int code);
    /*!
     * \brief Funkcia resetuje variáciu.
     *
     *  Nastavý kód a všetky prvky poľa mVariation na hodnotu 0.
     */
    void reset();
    void print();
    ~Variation();
};

#endif // VARIATION_H
