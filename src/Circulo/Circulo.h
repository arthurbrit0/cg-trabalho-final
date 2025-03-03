//
// Created by Vitor de Albuquerque on 16/12/2024.
//

#ifndef CIRCULO_H
#define CIRCULO_H
#include "funcoes_auxiliares.h"
using namespace funcoes_auxiliares;

struct Circulo
{
  Vetor3d centro;
  float raio;
  Vetor3d normal;

  Vetor3d K_d;
  Vetor3d K_e;
  Vetor3d K_a;
  float m;

  Circulo(Vetor3d centro,
          float raio,
          Vetor3d normal,
          Vetor3d K_d,
          Vetor3d K_e,
          Vetor3d K_a,
          float m);

  void transformar(Matriz mat);
};

#endif // CIRCULO_H
