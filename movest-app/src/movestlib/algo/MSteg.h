//
// Created by el on 01/01/16.
//

#ifndef MOVEST_MSTEG_H
#define MOVEST_MSTEG_H

#include "HideSeek.h"

class MSteg : public HideSeek {
public:
    bool embedIntoMvComponent(int16_t *mv, int bit);
    bool extractFromMvComponent(int16_t val, int *bit);
};


#endif //MOVEST_MSTEG_H
