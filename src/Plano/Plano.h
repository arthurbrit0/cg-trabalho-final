//
// Created by Vitor de Albuquerque on 16/12/2024.
//

#ifndef PLANO_H
#define PLANO_H
#include "funcoes_auxiliares.h"
using namespace funcoes_auxiliares;

struct Plano
{
  Vetor3d ponto;
  Vetor3d normal;
  Vetor3d K_d;
  Vetor3d K_e;
  Vetor3d K_a;
  float m;

  // Construtor correto para inicializar as propriedades do plano
  Plano(Vetor3d ponto,
        Vetor3d normal,
        Vetor3d K_d,
        Vetor3d K_e,
        Vetor3d K_a,
        float m);

  static Plano tres_pontos(Vetor3d P1,
                           Vetor3d P2,
                           Vetor3d P3,
                           Vetor3d K_d,
                           Vetor3d K_e,
                           Vetor3d K_a,
                           float m);

  Vetor3d calcular_iluminacao(Vetor3d Pt,
                              Vetor3d dr,
                              Vetor3d P_F,
                              Vetor3d I_F,
                              Vetor3d I_A);

  void transformar(Matriz mat);
};

#endif // PLANO_H
