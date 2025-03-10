#include <vector>
#include "funcoes_auxiliares.h"
#include "./src/Objeto/Objeto.h"
#include "./src/Raio/Raio.h"

std::pair<float,int> calcular_intersecao(Raio &raio,
                                         std::vector<Objeto> &objetos,
                                         int excluir)
{
  int objeto_hit = -1;
  float menor_t  = -1.0f;

  for (int i = 0; i < (int)objetos.size(); ++i) {
    if (i == excluir || !objetos[i].visivel) continue;

    float t = raio.intersecao(objetos[i]);
    if (t > 0.0f && (menor_t < 0.0f || t < menor_t)) {
      menor_t   = t;
      objeto_hit= i;
    }
  }

  return {menor_t, objeto_hit};
}
