//
// Created by el on 02/01/16.
//

#ifndef MOVEST_F4_H
#define MOVEST_F4_H

#include "HideSeek.h"

class F4 : public HideSeek {
protected:
    void embedIntoMvComponent(int16_t *mv);
    void extractFromMvComponent(int16_t val);
};

#endif //MOVEST_F4_H
