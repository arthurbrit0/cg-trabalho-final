#include "Objeto.h"

Objeto::Objeto(const Esfera& esfera)
  : obj(esfera)
{
  material = MaterialSimples(esfera.K_d, esfera.K_e, esfera.K_a, esfera.m);
  visivel = true;
}

Objeto::Objeto(const Plano& plano)
  : obj(plano)
{
  material = MaterialSimples(plano.K_d, plano.K_e, plano.K_a, plano.m);
  visivel = true;
}

Objeto::Objeto(const PlanoTextura& plano_tex)
  : obj(plano_tex)
{
  material = MaterialSimples({ 0.5f, 0.5f, 0.5f },
                             { 0.5f, 0.5f, 0.5f },
                             { 0.5f, 0.5f, 0.5f },
                             1.0f); // Define o material conforme necessário
  visivel = true;
}

Objeto::Objeto(const Cilindro& cilindro)
  : obj(cilindro)
{
  material =
    MaterialSimples(cilindro.K_d, cilindro.K_e, cilindro.K_a, cilindro.m);
  visivel = true;
}

Objeto::Objeto(const Cone& cone)
  : obj(cone)
{
  material = MaterialSimples(cone.K_d, cone.K_e, cone.K_a, cone.m);
  visivel = true;
}

Objeto::Objeto(const Circulo& circulo)
  : obj(circulo)
{
  material = MaterialSimples(circulo.K_d, circulo.K_e, circulo.K_a, circulo.m);
  visivel = true;
}

Objeto::Objeto(const Triangulo& triangulo)
  : obj(triangulo)
{
  material =
    MaterialSimples(triangulo.K_d, triangulo.K_e, triangulo.K_a, triangulo.m);
  visivel = true;
}

Objeto::Objeto(const Malha& malha)
  : obj(malha)
{
  material = malha.material;
  visivel = true;
}

// Implementação da função normal usando std::visit
Vetor3d
Objeto::normal(Vetor3d Pt)
{
  return std::visit(
    [Pt](auto&& objeto) -> Vetor3d {
      using T = std::decay_t<decltype(objeto)>;
      if constexpr (std::is_same_v<T, Esfera>)
        return objeto.normal(Pt);
      else if constexpr (std::is_same_v<T, Plano>)
        return objeto.normal;
      else if constexpr (std::is_same_v<T, PlanoTextura>)
        return objeto.normal;
      else if constexpr (std::is_same_v<T, Cilindro>)
        return objeto.normal(Pt);
      else if constexpr (std::is_same_v<T, Cone>)
        return objeto.normal(Pt);
      else if constexpr (std::is_same_v<T, Circulo>)
        return objeto.normal;
      else if constexpr (std::is_same_v<T, Triangulo>)
        return objeto.normal;
      else if constexpr (std::is_same_v<T, Malha>)
        return objeto.faces[objeto.triangulo_atingido].normal;
      else {
        TraceLog(LOG_INFO, "Problema: normal de objeto não suportada");
        return { 0.0f, 0.0f, 0.0f };
      }
    },
    obj);
}

// Implementação da função transformar usando std::visit
void
Objeto::transformar(Matriz mat)
{
  std::visit(
    [mat](auto&& objeto) {
      using T = std::decay_t<decltype(objeto)>;
      if constexpr (std::is_same_v<T, Esfera>)
        objeto.transformar(mat);
      else if constexpr (std::is_same_v<T, Plano>)
        objeto.transformar(mat);
      else if constexpr (std::is_same_v<T, PlanoTextura>)
        objeto.transformar(mat);
      else if constexpr (std::is_same_v<T, Cilindro>)
        objeto.transformar(mat);
      else if constexpr (std::is_same_v<T, Cone>)
        objeto.transformar(mat);
      else if constexpr (std::is_same_v<T, Circulo>)
        objeto.transformar(mat);
      else if constexpr (std::is_same_v<T, Triangulo>)
        objeto.transformar(mat);
      else if constexpr (std::is_same_v<T, Malha>)
        objeto.transformar(mat);
    },
    obj);
}
