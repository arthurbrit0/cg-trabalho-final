#ifndef OBJETO_COMPLEXO_H
#define OBJETO_COMPLEXO_H
#include "../Objeto/Objeto.h"
#include "funcoes_auxiliares.h"
#include <vector>

using namespace funcoes_auxiliares;

struct ObjetoComplexo
{
  std::vector<Objeto> objetos;
  std::vector<ObjetoComplexo> objetosComplexos;

  void transformar(Matriz mat);
  void adicionar_objeto(const Objeto& obj);

  Vetor3d normal(const Vetor3d& Pt) const;

  void adicionar_objeto_complexo(const ObjetoComplexo& obj_complexo);
};

#endif // OBJETO_COMPLEXO_H
