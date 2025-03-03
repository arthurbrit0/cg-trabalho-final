//
// Created by I590177 on 12/12/2024.
//

#ifndef CONE_H
#define CONE_H
#include "funcoes_auxiliares.h"
using namespace funcoes_auxiliares;

struct Cone
{
  Vetor3d centro;
  float raio;
  float altura;
  Vetor3d direcao;

  Vetor3d K_d;
  Vetor3d K_e;
  Vetor3d K_a;
  float m;

  Cone(Vetor3d centro,
       float raio,
       float altura,
       Vetor3d direcao,
       Vetor3d K_d,
       Vetor3d K_e,
       Vetor3d K_a,
       float m);

  Vetor3d calcular_iluminacao(Vetor3d Pt,
                              Vetor3d dr,
                              Vetor3d P_F,
                              Vetor3d I_F,
                              Vetor3d I_A);

  Vetor3d normal(Vetor3d Pt);
  void transformar(Matriz mat);
};

#endif // CONE_H
