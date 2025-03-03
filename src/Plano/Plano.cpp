//
// Created by Vitor de Albuquerque on 16/12/2024.
//
#include "Plano.h"
#include <math.h>

Plano::Plano(Vetor3d ponto,
             Vetor3d normal,
             Vetor3d K_d,
             Vetor3d K_e,
             Vetor3d K_a,
             float m)
  : ponto(ponto)
  , normal(normal)
  , K_d(K_d)
  , K_e(K_e)
  , K_a(K_a)
  , m(m)
{
}

Plano
Plano::tres_pontos(Vetor3d P1,
                   Vetor3d P2,
                   Vetor3d P3,
                   Vetor3d K_d,
                   Vetor3d K_e,
                   Vetor3d K_a,
                   float m)
{
  Vetor3d v1 = P2 - P1;
  Vetor3d v2 = P3 - P1;
  Vetor3d w = v1.cross_product(v2);
  Vetor3d n = w.normalizado();
  return Plano(P1, n, K_d, K_e, K_a, m);
}

Vetor3d
Plano::calcular_iluminacao(Vetor3d Pt,
                           Vetor3d dr,
                           Vetor3d P_F,
                           Vetor3d I_F,
                           Vetor3d I_A)
{
  Vetor3d v = dr * -1;
  Vetor3d l = (P_F - Pt).normalizado();
  float dotproduct_nl = normal.dot_product(l);
  Vetor3d r = 2 * dotproduct_nl * normal - l;
  float dotproduct_vr = v.dot_product(r);

  Vetor3d I_a = K_a * I_A;
  Vetor3d I_d = K_d * I_F * max(dotproduct_nl, 0.0);
  Vetor3d I_e = K_e * I_F * pow(max(dotproduct_vr, 0.0), m);

  Vetor3d I_total = I_d + I_e + I_a;
  return I_total;
}

void
Plano::transformar(Matriz mat)
{
  ponto = (mat * ponto.ponto4d()).vetor3d();
  normal = (mat * normal.vetor4d()).vetor3d();
}