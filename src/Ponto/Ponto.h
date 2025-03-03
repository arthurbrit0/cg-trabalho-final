//
// Created by Vitor de Albuquerque on 16/12/2024.
//

#ifndef PONTO_H
#define PONTO_H



struct Ponto {
    float x, y, z;

    // Construtor padrão
    Ponto(float x = 0.0f, float y = 0.0f, float z = 0.0f);

    // Operador de soma
    Ponto operator+(const Ponto& outro) const;

    // Operador de subtração
    Ponto operator-(const Ponto& outro) const;

    // Distância entre dois pontos
    float distancia(const Ponto& outro) const;
};



#endif //PONTO_H
