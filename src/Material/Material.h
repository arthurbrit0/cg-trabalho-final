#ifndef MATERIAL_H
#define MATERIAL_H

#include "funcoes_auxiliares.h"
using namespace funcoes_auxiliares;

	struct MaterialSimples
	{
 	 Vetor3d K_d;
 	 Vetor3d K_e;
 	 Vetor3d K_a;
  	 float m;

  	MaterialSimples() {}

  	MaterialSimples(Vetor3d K_d, Vetor3d K_e, Vetor3d K_a, float m);
	};

#endif // MATERIAL_H
