#ifndef VARIATION_H
#define VARIATION_H

class Variation
{
private:
    int mN;
    int mK;
    int mCode;
    int * mVariation;
    int mTotalVariations;
public:
    Variation(int n, int k);
    Variation(Variation & v);
    Variation();
    void set(Variation & v);
    int getN() { return mN; }
    int getK() { return mK; }
    int getTotalVariations() { return mTotalVariations; }
    bool next();
    int getCode();
    int * getVariation();
    void setCode(int code);
    void reset();
    void print();
    ~Variation();
};

#endif // VARIATION_H
