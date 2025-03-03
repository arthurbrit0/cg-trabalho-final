#include "calcular_intersecao.h"

std::pair<float, int>
calcular_intersecao(Raio& raio, std::vector<Objeto>& objetos, int excluir)
{
  int objeto = -1;
  float menor_t = -1.0f;

  for (int i = 0; i < objetos.size(); ++i) {
    if (i == excluir || !objetos[i].visivel)
      continue;

    float t;
    t = raio.intersecao(objetos[i]);
    if (t > 0.0f && (menor_t < 0.0f || t < menor_t)) {
      menor_t = t;
      objeto = i;
    }
  }

  return { menor_t, objeto };
}