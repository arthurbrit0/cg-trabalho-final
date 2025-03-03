//
// Created by Vitor de Albuquerque on 16/12/2024.
//

#include "Circulo.h"

Circulo::Circulo(Vetor3d centro,
                 float raio,
                 Vetor3d normal,
                 Vetor3d K_d,
                 Vetor3d K_e,
                 Vetor3d K_a,
                 float m)
  : centro(centro)
  , raio(raio)
  , normal(normal)
  , K_d(K_d)
  , K_e(K_e)
  , K_a(K_a)
  , m(m)
{
}

void
Circulo::transformar(Matriz mat)
{
  centro = (mat * centro.ponto4d()).vetor3d();
  normal = (mat * normal.vetor4d()).vetor3d();
}