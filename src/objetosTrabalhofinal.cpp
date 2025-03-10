//
// Created by Vitor on 01/02/2025.
// Cena: "Rua Comercial Vista de Perfil" – Versão ajustada:
// - Casas com telhado em cone menor (raio=70, altura=70), sem flutuar
// - Árvores, postes e mobiliário entre as fileiras de casas, na faixa da rua
// - Removido o anel suspenso para evitar cones extras flutuando
//

#include "objetosTrabalhofinal.h"
#include <cmath>
#include <iostream>
#include <raylib.h>
#include "inicializar_objetos.h"
#include "./Malha/Malha.h"
#include "./PlanoTextura/PlanoTextura.h"

void inicializar_objetosfinal(std::vector<Objeto>& objects_flat,
                              std::vector<ObjetoComplexo>& complexObjects)
{
  // 1) CHÃO (PlanoTextura)
  {
    Image textura_grama = LoadImage("assets/grama.png");
    Color* px_grama = LoadImageColors(textura_grama);
    if (!textura_grama.data) {
      std::cerr << "Erro ao carregar textura de grama.\n";
      return;
    }
    // Mapeia ~1000x1000
    Textura texChao(px_grama,
                    textura_grama.width,
                    textura_grama.height,
                    textura_grama.width,
                    textura_grama.height,
                    10.0f);

    PlanoTextura chao({ 0.0f, -5.0f, 0.0f },  // ponto
                      { 1.0f, 0.0f, 0.0f },  // eixo1
                      { 0.0f, 0.0f, 1.0f },  // eixo2
                      texChao);
    objects_flat.push_back(chao);
    std::cout << "Chão de grama inicializado.\n";
  }

  // 2) CÉU (PlanoTextura)
  {
    Image textura_ceu = LoadImage("assets/ceu2.png");
    Color* px_ceu = LoadImageColors(textura_ceu);
    if (!textura_ceu.data) {
      std::cerr << "Erro ao carregar textura do céu.\n";
      return;
    }
    Textura texCeu(px_ceu,
                   textura_ceu.width,
                   textura_ceu.height,
                   textura_ceu.width / 0.01f,
                   textura_ceu.height / 0.01f,
                   1.0f);

    PlanoTextura ceu({ 1000.0f, 250.0f, -20000.0f },
                     { 1.0f, 0.0f, 0.0f },
                     { 0.0f, 1.0f, 0.0f },
                     texCeu);
    objects_flat.push_back(ceu);
    std::cout << "Céu inicializado.\n";
  }

  // 3) RUA (Malha)
  {
    Malha rua;
    Vetor3d corRua = { 0.15f, 0.15f, 0.15f };
    rua.inicializar_cubo({ 0, 0, 0 }, 1.0f,
                          corRua, corRua, corRua,
                          32.0f);
    // Largura 1000 (de x=-500 a x=+500) e comprimento 4000, altura mínima
    rua.transformar(Matriz::translacao({ 0.0f, 0.0f, 0.0f }) *
                    Matriz::escala({ 1600.0f, 10.0f, 5000.0f }));
    objects_flat.push_back(rua);
    std::cout << "Rua pavimentada inicializada.\n";
  }

  // Função lambda para criar uma casa (cubo + cone de telhado)
  auto criarCasa = [&](float posX, float posZ, Vetor3d corCorpo, Vetor3d corTelhado) -> ObjetoComplexo {
    ObjetoComplexo casa;

    // Corpo da casa (cubo)
    Malha corpo;
    corpo.inicializar_cubo({ 0, 0, 0 }, 1.0f,
                           corCorpo, corCorpo, corCorpo,
                           32.0f);
    // 150(larg) x 250(alt) x 150(prof)
    corpo.transformar(Matriz::translacao({ posX, 0.0f, posZ }) *
                      Matriz::escala({ 150.0f, 250.0f, 150.0f }));
    casa.adicionar_objeto(corpo);

    // Telhado (cone) menor
    Cone telhado({ posX, 120.0f, posZ },
                 120.0f,
                 120.0f,
                 { 0, 1, 0 },
                 corTelhado, corTelhado, { 0.1f, 0.1f, 0.1f },
                 32.0f);
    casa.adicionar_objeto(telhado);

    return casa;
  };

  // 4) CASAS: duas filas
  for (int i = 0; i < 3; i++) {
    float zPos = -1500.0f + i * 800.0f;
    // Casa esquerda
    ObjetoComplexo casaEsq = criarCasa(-700.0f, zPos,
                                       {0.8f, 0.85f, 0.9f},
                                       {0.2f, 0.2f, 0.8f});
    complexObjects.push_back(casaEsq);
    // Casa direita
    ObjetoComplexo casaDir = criarCasa(700.0f, zPos,
                                       {0.95f, 0.9f, 0.8f},
                                       {0.9f, 0.5f, 0.2f});
    complexObjects.push_back(casaDir);
  }
  std::cout << "Casas alinhadas inicializadas.\n";

  // 5) POSTES
  auto criarPoste = [&](float x, float z) -> ObjetoComplexo {
    ObjetoComplexo poste;
    Cilindro base(
      { x, 0.0f, z },
      3.0f,
      100.0f,
      { 0,1,0 },
      { 0.4f,0.4f,0.4f },
      { 0.4f,0.4f,0.4f },
      { 0.1f,0.1f,0.1f },
      16.0f
    );
    Cone lampada(
      { x, 100.0f, z },
      6.0f,
      20.0f,
      { 0,1,0 },
      { 1.0f,1.0f,0.9f },
      { 1.0f,1.0f,0.9f },
      { 0.1f,0.1f,0.1f },
      32.0f
    );
    poste.adicionar_objeto(base);
    poste.adicionar_objeto(lampada);
    return poste;
  };

  for (int i = 0; i < 5; i++) {
    float zPos = -1500.0f + i * 600.0f;
    // Poste esq
    complexObjects.push_back(criarPoste(-300.0f, zPos));
    // Poste dir
    complexObjects.push_back(criarPoste(300.0f, zPos));
  }
  std::cout << "Postes alinhados inicializados.\n";

  // 6) ÁRVORES COM MAÇÃS
  auto criarArvore = [&](float x, float z) -> ObjetoComplexo {
    ObjetoComplexo arvore;
    // Tronco
    Cilindro tronco({ x, 0.0f, z }, 5.0f, 120.0f, {0,1,0},
                    { 0.5f, 0.25f, 0.1f },
                    { 0.5f, 0.25f, 0.1f },
                    { 0.1f, 0.1f, 0.1f },
                    16.0f);
    arvore.adicionar_objeto(tronco);

    // Copa
    Cone copa({ x, 120.0f, z }, 70.0f, 100.0f, {0,1,0},
              {0.0f, 0.6f, 0.0f},
              {0.0f, 0.6f, 0.0f},
              {0.1f, 0.1f, 0.1f},
              16.0f);
    arvore.adicionar_objeto(copa);

    // Maçãs
    for (int i = 0; i < 3; i++) {
      float offsetX = (i - 1)*20.0f;
      float offsetZ = (i == 1) ? 0.0f : (i == 0 ? 20.0f : -20.0f);
      float offsetY = 180.0f + i*10.0f;

      Esfera maca({ x+offsetX, offsetY, z+offsetZ },
                  15.0f,
                  {1.0f, 0.0f, 0.0f},
                  {1.0f, 1.0f, 1.0f},
                  {0.1f, 0.1f, 0.1f},
                  16.0f);
      arvore.adicionar_objeto(maca);
    }
    return arvore;
  };

  for (int i = 0; i < 5; i++) {
    float zPos = -1300.0f + i * 500.0f;
    complexObjects.push_back(criarArvore(-400.0f, zPos));
    complexObjects.push_back(criarArvore( 400.0f, zPos));
  }
  std::cout << "Árvores alinhadas inicializadas.\n";
  std::cout << "\nCena 'Rua Comercial Vista de Perfil' ajustada com sucesso!\n";

  // 10) Faixa amarela tracejada
  {
    auto criarTraco = [&](float z) -> ObjetoComplexo {
      ObjetoComplexo traco;
      Malha dash;
      dash.inicializar_cubo({ 0, 0, 0 }, 1.0f,
                            { 1.0f, 1.0f, 0.0f },
                            { 1.0f, 1.0f, 0.0f },
                            { 1.0f, 1.0f, 0.0f },
                            32.0f);
      // 50 comp x 5 larg x 1 alt => no transform adaptado
      dash.transformar(Matriz::translacao({ 0.0f, 5.5f, z }) *
                       Matriz::escala({ 50.0f, 1.0f, 120.0f }));
      traco.adicionar_objeto(dash);
      return traco;
    };

    for (int i = 0; i < 10; i++) {
      float zPos = -2000.0f + i * 400.0f;
      complexObjects.push_back(criarTraco(zPos));
    }
    std::cout << "Faixa amarela tracejada adicionada ao centro da rua.\n";
  }

}
