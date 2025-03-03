#ifndef CALCULAR_INTERSECAO_H
#define CALCULAR_INTERSECAO_H

#include "./Objeto/Objeto.h"
#include "./Raio/Raio.h"
#include <vector>

std::pair<float, int>
calcular_intersecao(Raio& raio, std::vector<Objeto>& objetos, int excluir = -1);

#endif