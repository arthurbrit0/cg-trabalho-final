//
// Created by I590177 on 12/12/2024.
//

#ifndef ARESTA_H
#define ARESTA_H
#include "funcoes_auxiliares.h"

using namespace funcoes_auxiliares;

struct Aresta
{
  Vetor3d p1, p2; // Pontos conectados pela aresta

  // Construtor padrão
  Aresta(const Vetor3d& p1, const Vetor3d& p2);

  float comprimento() const;
};

#endif // ARESTA_H
