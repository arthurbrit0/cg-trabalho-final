#include "Esfera.h"
#include <cmath>

// Construtor parametrizado
// Definição do construtor da Esfera
Esfera::Esfera(Vetor3d centro,
               float raio,
               Vetor3d K_d,
               Vetor3d K_e,
               Vetor3d K_a,
               float m)
  : centro(centro)
  , raio(raio)
  , K_d(K_d)
  , K_e(K_e)
  , K_a(K_a)
  , m(m)
{
}

Vetor3d
Esfera::calcular_iluminacao(Vetor3d Pt,
                            Vetor3d dr,
                            Vetor3d P_F,
                            Vetor3d I_F,
                            Vetor3d I_A)
{
  Vetor3d v = dr * -1;
  Vetor3d l = (P_F - Pt).normalizado();
  Vetor3d n = (Pt - centro).normalizado();
  float dotproduct_nl = n.dot_product(l);
  Vetor3d r = 2 * dotproduct_nl * n - l;
  float dotproduct_vr = v.dot_product(r);

  Vetor3d I_d = K_d * I_F * max(dotproduct_nl, 0);
  Vetor3d I_e = K_e * I_F * pow(max(dotproduct_vr, 0), m);
  Vetor3d I_a = K_a * I_A;

  Vetor3d I_total = I_d + I_e + I_a;
  return I_total;
}

Vetor3d
Esfera::normal(Vetor3d Pt)
{
  return (Pt - centro).normalizado();
}

void
Esfera::transformar(Matriz mat)
{
  Vetor3d ponto = centro + (Vetor3d){ raio, 0.0f, 0.0f };
  Vetor3d novo_ponto = (mat * ponto.ponto4d()).vetor3d();
  Vetor3d novo_centro = (mat * centro.ponto4d()).vetor3d();
  float novo_raio = (novo_ponto - novo_centro).tamanho();
  raio = novo_raio;
  centro = novo_centro;
}