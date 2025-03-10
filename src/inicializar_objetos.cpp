#include <iostream>
#include <raylib.h>
#include "funcoes_auxiliares.h"
#include "./src/Raio/Raio.h"
#include "./src/Objeto/Objeto.h"
#include "./src/Esfera/Esfera.h"
#include "./src/Cilindro/Cilindro.h"
#include "./src/Cone/Cone.h"
#include "./src/Malha/Malha.h"
#include "./src/PlanoTextura/PlanoTextura.h"
#include "./src/Textura/Textura.h"
#include <stack>

#include "calcular_intersecao.h"
#include "inicializar_objetos.h"

// Exemplo simples (se quiser usar). Sem spot, sem cisalhamento, etc.
void inicializar_objetos(std::vector<Objeto> &objects_flat)
{
  // Carrega imagem p/ texturizar um plano
  Image img = LoadImage("assets/grama.png");
  if (!img.data) {
    std::cerr << "Erro ao carregar grama.png!\n";
    return;
  }
  Color* pixels = LoadImageColors(img);

  Textura tex(
    pixels,
    img.width,
    img.height,
    (float)img.width,
    (float)img.height,
    8.0f
  );

  // Plano texturizado
  PlanoTextura chao(
    {0.0f, 0.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f},
    tex
  );
  objects_flat.push_back(Objeto(chao));

  // Exemplo de esfera
  Esfera esf(
    {30.0f, 20.0f, 30.0f},
    20.0f,
    {1.0f, 0.2f, 0.2f},
    {1.0f, 1.0f, 1.0f},
    {0.2f, 0.2f, 0.2f},
    16.0f
  );
  objects_flat.push_back(Objeto(esf));

  // etc...
  // (Pode deixar do jeito que estava ou vazio)
  UnloadImageColors(pixels);
  UnloadImage(img);

  std::cout << "Cena inicializada (exemplo)! Objetos: " << objects_flat.size() << "\n";
}

void flatten_objetos(ObjetoComplexo &obj_complexo,
                     std::vector<Objeto> &objetos_flat)
{
  // Se quiser a mesma lógica de flatten:
  std::stack<ObjetoComplexo*> pilha;
  pilha.push(&obj_complexo);
  while(!pilha.empty()) {
    ObjetoComplexo *atual = pilha.top();
    pilha.pop();
    for (auto &obj : atual->objetos) {
      objetos_flat.push_back(obj);
    }
    for (auto &sub : atual->objetosComplexos) {
      pilha.push(&sub);
    }
  }
}

void deletar_objetos(Color *pixels_textura, Image textura)
{
  UnloadImageColors(pixels_textura);
  UnloadImage(textura);
}
