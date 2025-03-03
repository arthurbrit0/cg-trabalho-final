#ifndef PLANO_TEXTURA_H
#define PLANO_TEXTURA_H

#include "./../Material/Material.h"
#include "./../Textura/Textura.h"
#include "funcoes_auxiliares.h"
using namespace funcoes_auxiliares;

// Representa um plano com textura
struct PlanoTextura
{
  Vetor3d ponto;
  Vetor3d normal;
  Vetor3d eixo1;
  Vetor3d eixo2;
  Textura textura;

  // Construtor do plano. Aceita um ponto do plano, que será sua origem
  // e também aceita dois vetores que representam o sistema de coordenadas
  // do plano. Assume-se que eles são ortonormais. Além disso, aceita
  // a textura do plano.
  PlanoTextura(Vetor3d ponto, Vetor3d eixo1, Vetor3d eixo2, Textura textura);

  MaterialSimples material(Vetor3d Pt);

  void transformar(Matriz mat);
};

#endif // PLANO_TEXTURA_H