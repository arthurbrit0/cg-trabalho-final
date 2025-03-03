#ifndef RAIO_H
#define RAIO_H

#include "./../Cilindro/Cilindro.h"
#include "./../Circulo/Circulo.h"
#include "./../Cone/Cone.h"
#include "./../Esfera/Esfera.h"
#include "./../Malha/Malha.h"
#include "./../Objeto/Objeto.h"
#include "./../Plano/Plano.h"
#include "./../PlanoTextura/PlanoTextura.h"
#include "./../Triangulo/Triangulo.h"
#include "funcoes_auxiliares.h"
#include <vector>
using namespace funcoes_auxiliares;
class Objeto; // Declaração antecipada

struct Raio
{
  Vetor3d P0;
  Vetor3d dr;

  Raio(Vetor3d P0, Vetor3d dr);
  Vetor3d no_ponto(float t);
  float intersecao(Objeto& objeto);
  float intersecao(const Esfera& esfera);
  float intersecao(const Plano& plano); // Função de intersecção com Plano
  float intersecao(const PlanoTextura& plano_tex);
  float intersecao(const Cone& cone);
  float intersecao(const Cilindro& cilindro);
  float intersecao(const Circulo& circulo);
  float intersecao(const Triangulo& triangulo);
  float intersecao(Malha& malha);
};

// calculo do material para calcular iluminação

#endif // RAIO_H
