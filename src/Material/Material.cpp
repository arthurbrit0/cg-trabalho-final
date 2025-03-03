#include "Material.h"


// Construtor padrão
MaterialSimples::MaterialSimples(Vetor3d K_d, Vetor3d K_e, Vetor3d K_a, float m)
  : K_d(K_d)
  , K_e(K_e)
  , K_a(K_a)
  , m(m)
{
}