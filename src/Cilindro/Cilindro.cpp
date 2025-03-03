//
// Created by I590177 on 12/12/2024.
//

#include "Cilindro.h"
#include <math.h>

Cilindro::Cilindro(Vetor3d centro,
                   float raio,
                   float altura,
                   Vetor3d direcao,
                   Vetor3d K_d,
                   Vetor3d K_e,
                   Vetor3d K_a,
                   float m)
  : centro(centro)
  , raio(raio)
  , altura(altura)
  , direcao(direcao)
  , K_d(K_d)
  , K_e(K_e)
  , K_a(K_a)
  , m(m)
{
}

Vetor3d
Cilindro::calcular_iluminacao(Vetor3d Pt,
                              Vetor3d dr,
                              Vetor3d P_F,
                              Vetor3d I_F,
                              Vetor3d I_A)
{
  Vetor3d z = direcao;
  Vetor3d centro_Pt = Pt - centro;
  Vetor3d normal = (centro_Pt - centro_Pt.dot_product(z) * z).normalizado();
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

Vetor3d
Cilindro::normal(Vetor3d Pt)
{
  Vetor3d z = direcao;
  Vetor3d centro_Pt = Pt - centro;
  return (centro_Pt - centro_Pt.dot_product(z) * z).normalizado();
}

void
Cilindro::transformar(Matriz mat)
{
  Vetor3d topo = centro + altura * direcao;
  centro = (mat * centro.ponto4d()).vetor3d();
  topo = (mat * topo.ponto4d()).vetor3d();
  direcao = (mat * direcao.vetor4d()).vetor3d();
  altura = (topo - centro).tamanho();
}