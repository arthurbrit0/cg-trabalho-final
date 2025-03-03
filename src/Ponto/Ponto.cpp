//
// Created by Vitor de Albuquerque on 16/12/2024.
//
#include <math.h>

#include "Ponto.h"
Ponto::Ponto(float x, float y, float z) : x(x), y(y), z(z) {}

Ponto Ponto::operator+(const Ponto& outro) const {
    return {x + outro.x, y + outro.y, z + outro.z};
}

Ponto Ponto::operator-(const Ponto& outro) const {
    return {x - outro.x, y - outro.y, z - outro.z};
}

float Ponto::distancia(const Ponto& outro) const {
    return std::sqrt((x - outro.x) * (x - outro.x) +
                     (y - outro.y) * (y - outro.y) +
                     (z - outro.z) * (z - outro.z));
}