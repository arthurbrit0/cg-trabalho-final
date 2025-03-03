//
// Created by Vitor de Albuquerque on 16/12/2024.
//

#ifndef TRIANGULO_H
#define TRIANGULO_H
#include "funcoes_auxiliares.h"
using namespace funcoes_auxiliares;

struct Triangulo
{

  Vetor3d P0;
  Vetor3d P1;
  Vetor3d P2;

  Vetor3d normal;
  float area2; // dobro da area
  Vetor3d lado1;
  Vetor3d lado2;
  Vetor3d lado3;

  Vetor3d K_d;
  Vetor3d K_e;
  Vetor3d K_a;
  float m;

  Triangulo(Vetor3d P0,
            Vetor3d P1,
            Vetor3d P2,
            Vetor3d K_d,
            Vetor3d K_e,
            Vetor3d K_a,
            float m);

  void transformar(Matriz mat);
};

#endif // TRIANGULO_H
