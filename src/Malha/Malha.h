#ifndef MALHA_H
#define MALHA_H

#include "./funcoes_auxiliares.h" // Certifique-se de incluir Vetor3d
#include "./src/Aresta/Aresta.h"
#include "./src/Material/Material.h"
#include "./src/Triangulo/Triangulo.h"
#include <vector>
#include <cstddef>


struct Malha
{
  Vetor3d ancora;
  std::vector<Vetor3d> vertices; // Lista de vértices
  std::vector<Aresta> arestas;   // Lista de arestas
  std::vector<Triangulo> faces;  // Lista de faces (Triângulos)
  MaterialSimples material;
  size_t triangulo_atingido = -1;

  Malha(); // Construtor padrão
  void inicializar_cubo(const Vetor3d& centro,
                        float tamanho,
                        const Vetor3d& K_d,
                        const Vetor3d& K_e,
                        const Vetor3d& K_a,
                        float m);
  void imprimir();

  void transformar(Matriz mat);
};

#endif // MALHA_H