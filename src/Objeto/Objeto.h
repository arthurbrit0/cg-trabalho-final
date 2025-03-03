#ifndef OBJETO_H
#define OBJETO_H

#include "./src/Material/Material.h"
#include "funcoes_auxiliares.h"
#include <variant>

// Inclua os headers necessários (sem criar loops)
#include "./../Cilindro/Cilindro.h"
#include "./../Circulo/Circulo.h"
#include "./../Cone/Cone.h"
#include "./../Esfera/Esfera.h"
#include "./../Malha/Malha.h"
#include "./../Plano/Plano.h"
#include "./../PlanoTextura/PlanoTextura.h"
#include "./../Triangulo/Triangulo.h"

// Estrutura Objeto
struct Objeto
{
  std::variant<Esfera,
               Plano,
               PlanoTextura,
               Cilindro,
               Cone,
               Circulo,
               Triangulo,
               Malha>
    obj;
  MaterialSimples material;
  bool visivel;

  // Construtores para cada tipo de objeto
  Objeto(const Esfera& esfera);
  Objeto(const Plano& plano);
  Objeto(const PlanoTextura& plano_tex);
  Objeto(const Cilindro& cilindro);
  Objeto(const Cone& cone);
  Objeto(const Circulo& circulo);
  Objeto(const Triangulo& triangulo);
  Objeto(const Malha& malha);

  Vetor3d normal(Vetor3d Pt);
  void transformar(Matriz mat);
};

#endif // OBJETO_H
