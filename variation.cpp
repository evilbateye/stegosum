#include "variation.h"
#include "qmath.h"
#include <iostream>

Variation::Variation(int n, int k) : mN(n), mK(k), mCode(0) {
    mVariation = new int[k];
    std::fill_n(mVariation, k, 0);
    mTotalVariations = qPow(mN, mK);
}

bool Variation::next() {

    mCode++;

    if (mCode >= mTotalVariations) return false;

    for (int k = 0; k < mK; k++) {
        mVariation[k]++;

        if (mVariation[k] < mN) break;

        mVariation[k] = 0;
    }

    return true;
}

int Variation::getCode() {
    return mCode;
}

int * Variation::getVariation() {
    return mVariation;
}

void Variation::setCode(int code) {

    this->reset();

    while (mCode != code) this->next();
}

void Variation::reset() {
    mCode = 0;
    std::fill_n(mVariation, mK, 0);
}

void Variation::print() {
    std::cout << "variation " << mCode << " :";
    for (int i = 0; i < mK; i++) std::cout << " " << mVariation[i];
    std::cout << std::endl;
}

Variation::~Variation() {
    if (mVariation) delete mVariation;
}
