//
// Created by I590177 on 12/12/2024.
//

#include "Aresta.h"
#include <math.h>

Aresta::Aresta(const Vetor3d& p1, const Vetor3d& p2) : p1(p1), p2(p2) {}

float Aresta::comprimento() const {
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) +
                (p1.y - p2.y) * (p1.y - p2.y) +
                (p1.z - p2.z) * (p1.z - p2.z));
}