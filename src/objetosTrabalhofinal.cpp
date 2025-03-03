//
// Created by Vitor on 01/02/2025.
// Cena: "Rua Comercial" – Uma cena cotidiana de uma rua urbana com prédios, poste, árvore, banco e abrigo de ônibus
//

#include "objetosTrabalhofinal.h"
#include <cmath>
#include <iostream>
#include <raylib.h>

#include "./Malha/Malha.h"
#include "./PlanoTextura/PlanoTextura.h"
#include "inicializar_objetos.h"

void inicializar_objetosfinal(std::vector<Objeto>& objects_flat,
                              std::vector<ObjetoComplexo>& complexObjects)
{
  // 1) CHÃO – Usamos a textura de grama para o piso da rua (PlanoTextura)
  {
    Image textura_grama = LoadImage("assets/grama.png");
    Color* pixels_textura_grama = LoadImageColors(textura_grama);
    if (!textura_grama.data) {
      std::cerr << "Erro ao carregar textura de grama.\n";
      return;
    }
    // Mapeia a textura para uma área ampla (1000x1000 unidades)
    Textura texturaG(pixels_textura_grama,
                     textura_grama.width,
                     textura_grama.height,
                     textura_grama.width,
                     textura_grama.height,
                     10.0f);
    // Posiciona o chão levemente abaixo da origem; a normal calculada é (0,-1,0)
    PlanoTextura chao({ 0.0f, -5.0f, 0.0f },
                       { 1.0f, 0.0f, 0.0f },
                       { 0.0f, 0.0f, 1.0f },
                       texturaG);
    objects_flat.push_back(chao);
    std::cout << "Chão de grama inicializado.\n";
  }

  // 2) CÉU – Mantido como fundo (como no arquivo original)
  {
    Image textura_ceu = LoadImage("assets/ceu2.png");
    Color* pixels_textura_ceu = LoadImageColors(textura_ceu);
    if (!textura_ceu.data) {
      std::cerr << "Erro ao carregar textura do céu.\n";
      return;
    }
    Textura texturaC(pixels_textura_ceu,
                     textura_ceu.width,
                     textura_ceu.height,
                     textura_ceu.width / 0.01f,
                     textura_ceu.height / 0.01f,
                     1.0f);
    PlanoTextura ceu({ 1000.0f, 250.0f, -20000.0f },
                     { 1.0f, 0.0f, 0.0f },
                     { 0.0f, 1.0f, 0.0f },
                     texturaC);
    objects_flat.push_back(ceu);
    std::cout << "Céu inicializado.\n";
  }

  // 3) PRÉDIO COMERCIAL – Um prédio simples feito com um cubo (Malha)
  {
    Malha predioComercial;
    predioComercial.inicializar_cubo({ 0, 0, 0 },
                                      1.0f,
                                      { 0.8f, 0.8f, 0.8f },
                                      { 0.8f, 0.8f, 0.8f },
                                      { 0.8f, 0.8f, 0.8f },
                                      32);
    // Reposiciona e escala para formar um prédio de 150x300x150 unidades
    predioComercial.transformar(Matriz::translacao({ 200.0f, 0.0f, 300.0f }) *
                                Matriz::escala({ 150.0f, 300.0f, 150.0f }));
    ObjetoComplexo comercial;
    comercial.adicionar_objeto(predioComercial);
    complexObjects.push_back(comercial);
    std::cout << "Prédio comercial inicializado.\n";
  }

  // 4) PRÉDIO RESIDENCIAL – Um prédio com telhado triangular
  {
    Malha predioResidencial;
    predioResidencial.inicializar_cubo({ 0, 0, 0 },
                                      1.0f,
                                      { 0.7f, 0.7f, 0.7f },
                                      { 0.7f, 0.7f, 0.7f },
                                      { 0.7f, 0.7f, 0.7f },
                                      32);
    predioResidencial.transformar(Matriz::translacao({ 600.0f, 0.0f, 300.0f }) *
                                  Matriz::escala({ 150.0f, 250.0f, 150.0f }));
    // Telhado em forma de triângulo
    Triangulo telhado(
      {525.0f, 250.0f, 375.0f}, {675.0f, 250.0f, 375.0f}, {600.0f, 350.0f, 375.0f},
      {0.9f, 0.2f, 0.2f}, {0.9f, 0.2f, 0.2f}, {0.1f, 0.1f, 0.1f},
      32.0f
    );
    ObjetoComplexo residencial;
    residencial.adicionar_objeto(predioResidencial);
    residencial.adicionar_objeto(telhado);
    complexObjects.push_back(residencial);
    std::cout << "Prédio residencial inicializado.\n";
  }

  // 5) POSTE DE ILUMINAÇÃO – Representado por um cilindro e um cone (lampada)
  {
    ObjetoComplexo poste;
    Cilindro posteBase(
      { 100.0f, 0.0f, 800.0f },
      2.0f,
      100.0f,
      { 0, 1, 0 },
      { 0.4f, 0.4f, 0.4f },
      { 0.4f, 0.4f, 0.4f },
      { 0.1f, 0.1f, 0.1f },
      16.0f
    );
    Cone lampada(
      { 100.0f, 100.0f, 800.0f },
      6.0f,
      15.0f,
      { 0, 1, 0 },
      { 1.0f, 1.0f, 0.8f },
      { 1.0f, 1.0f, 0.8f },
      { 0.1f, 0.1f, 0.1f },
      32.0f
    );
    poste.adicionar_objeto(posteBase);
    poste.adicionar_objeto(lampada);
    complexObjects.push_back(poste);
    std::cout << "Poste de iluminação inicializado.\n";
  }

  // 6) ÁRVORE – Com tronco (cilindro) e copa (cone)
  {
    ObjetoComplexo arvore;
    Cilindro tronco(
      { 400.0f, 0.0f, 800.0f },
      5.0f,
      120.0f,
      { 0, 1, 0 },
      { 0.5f, 0.3f, 0.1f },
      { 0.5f, 0.3f, 0.1f },
      { 0.1f, 0.1f, 0.1f },
      16.0f
    );
    Cone copa(
      { 400.0f, 120.0f, 800.0f },
      40.0f,
      80.0f,
      { 0, 1, 0 },
      { 0.0f, 0.6f, 0.0f },
      { 0.0f, 0.6f, 0.0f },
      { 0.1f, 0.1f, 0.1f },
      16.0f
    );
    arvore.adicionar_objeto(tronco);
    arvore.adicionar_objeto(copa);
    complexObjects.push_back(arvore);
    std::cout << "Árvore inicializada.\n";
  }

  // 7) BANCO – Um assento urbano composto por um cubo para o assento e dois cilindros como suportes
  {
    ObjetoComplexo banco;
    Malha assento;
    assento.inicializar_cubo({ 0, 0, 0 },
                              1.0f,
                              { 0.4f, 0.2f, 0.1f },
                              { 0.4f, 0.2f, 0.1f },
                              { 0.4f, 0.2f, 0.1f },
                              32.0f);
    assento.transformar(Matriz::translacao({ 600.0f, 0.0f, 900.0f }) *
                         Matriz::escala({ 100.0f, 10.0f, 40.0f }));
    banco.adicionar_objeto(assento);
    // Suportes
    Cilindro suporte1(
      { 550.0f, 0.0f, 900.0f },
      2.0f,
      20.0f,
      { 0,1,0 },
      { 0.4f,0.4f,0.4f },
      { 0.4f,0.4f,0.4f },
      { 0.1f,0.1f,0.1f },
      16.0f
    );
    Cilindro suporte2(
      { 650.0f, 0.0f, 900.0f },
      2.0f,
      20.0f,
      { 0,1,0 },
      { 0.4f,0.4f,0.4f },
      { 0.4f,0.4f,0.4f },
      { 0.1f,0.1f,0.1f },
      16.0f
    );
    banco.adicionar_objeto(suporte1);
    banco.adicionar_objeto(suporte2);
    complexObjects.push_back(banco);
    std::cout << "Banco inicializado.\n";
  }

  // 8) ABRIGO DE ÔNIBUS – Um pequeno pavilhão para proteção
  {
    ObjetoComplexo abrigo;
    Malha estrutura;
    estrutura.inicializar_cubo({ 0, 0, 0 },
                                1.0f,
                                { 0.8f, 0.8f, 0.8f },
                                { 0.8f, 0.8f, 0.8f },
                                { 0.8f, 0.8f, 0.8f },
                                32.0f);
    // Base do abrigo
    estrutura.transformar(Matriz::translacao({ 900.0f, 0.0f, 600.0f }) *
                          Matriz::escala({ 150.0f, 5.0f, 100.0f }));
    abrigo.adicionar_objeto(estrutura);
    // Teto do abrigo
    Malha teto;
    teto.inicializar_cubo({ 0, 0, 0 },
                           1.0f,
                           { 0.3f, 0.3f, 0.3f },
                           { 0.3f, 0.3f, 0.3f },
                           { 0.3f, 0.3f, 0.3f },
                           32.0f);
    teto.transformar(Matriz::translacao({ 900.0f, 50.0f, 600.0f }) *
                     Matriz::escala({ 150.0f, 10.0f, 100.0f }));
    abrigo.adicionar_objeto(teto);
    complexObjects.push_back(abrigo);
    std::cout << "Abrigo de ônibus inicializado.\n";
  }

  // 9) SINAL DE RUA – Um cone representando uma placa de sinalização
  {
    ObjetoComplexo sinal;
    Cone placa(
      { 750.0f, 80.0f, 800.0f },
      10.0f,
      30.0f,
      { 0,1,0 },
      { 1.0f,0.8f,0.0f },
      { 1.0f,0.8f,0.0f },
      { 0.1f,0.1f,0.1f },
      32.0f
    );
    sinal.adicionar_objeto(placa);
    complexObjects.push_back(sinal);
    std::cout << "Sinal de rua inicializado.\n";
  }

  // 10) ANEL SUSPENSO – Uma peça de arte cinética reinterpretada a partir da cesta de basquete
  {
    auto criarAnel = [&](float x, float z) -> ObjetoComplexo {
      ObjetoComplexo anel;
      Cilindro aro1({ x, 0.0f, z },
                    30.0f,
                    10.0f,
                    { 0,1,0 },
                    { 1.0f,0.5f,0.0f },
                    { 1.0f,0.5f,0.0f },
                    { 0.2f,0.1f,0.0f },
                    16.0f);
      Cilindro aro2({ x, 0.0f, z },
                    30.0f,
                    10.0f,
                    { 0,1,0 },
                    { 1.0f,0.5f,0.0f },
                    { 1.0f,0.5f,0.0f },
                    { 0.2f,0.1f,0.0f },
                    16.0f);
      anel.adicionar_objeto(aro1);
      anel.adicionar_objeto(aro2);
      return anel;
    };
    ObjetoComplexo anelSuspenso = criarAnel(650.0f, 700.0f);
    anelSuspenso.transformar(Matriz::translacao({ 0.0f, 100.0f, 0.0f }));
    complexObjects.push_back(anelSuspenso);
    std::cout << "Anel suspenso inicializado.\n";
  }

  std::cout << "\nCena 'Rua Comercial' inicializada com sucesso!\n";
}
