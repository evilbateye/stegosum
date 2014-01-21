#ifndef VARIATION_H
#define VARIATION_H

class Variation
{
private:
    int * mVariation;
    int mN;
    int mK;
    int mCode;
    int mTotalVariations;
public:
    Variation(int n, int k);
    bool next();
    int getCode();
    int * getVariation();
    void setCode(int code);
    void reset();
    void print();
    ~Variation();
};

#endif // VARIATION_H
