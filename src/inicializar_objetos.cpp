#include <iostream>
#include <raylib.h>
#include <stack>
#include <math.h>

#include "inicializar_objetos.h"
#include "./PlanoTextura/PlanoTextura.h"
#include "./Malha/Malha.h"

void inicializar_objetos(std::vector<Objeto>& objects_flat, std::vector<ObjetoComplexo>& complexObjects) {
Image textura_grama = LoadImage("assets/grama.png");
  Color* pixels_textura_grama = LoadImageColors(textura_grama);
  if (!textura_grama.data) {
    std::cerr << "Erro ao carregar textura de grama.\n";
    return;
  }
  Textura texturaG(pixels_textura_grama,
                   textura_grama.width,
                   textura_grama.height,
                   textura_grama.width,
                   textura_grama.height,
                   10.0f);

  PlanoTextura plano_grama(
    { 0.0f, -151.0f, 0.0f }, // Um pouco abaixo do “piso do galpão”
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    texturaG);
  objects_flat.push_back(plano_grama);
  std::cout << "Inicializou plano da grama.\n";

  Image textura_ceu = LoadImage("assets/ceu.png");
  Color* pixels_textura_ceu = LoadImageColors(textura_ceu);
  if (!textura_ceu.data) {
    std::cerr << "Erro ao carregar textura do céu.\n";
    return;
  }
  Textura texturaC(pixels_textura_ceu,
                   textura_ceu.width,
                   textura_ceu.height,
                   textura_ceu.width / 0.1f,
                   textura_ceu.height / 0.1f,
                   1.0f);

  PlanoTextura plano_ceu({ 300.0f, 250.0f, -10000.0f },
                         { 1.0f, 0.0f, 0.0f },
                         { 0.0f, 1.0f, 0.0f },
                         texturaC);
  objects_flat.push_back(plano_ceu);
  std::cout << "Inicializou plano do céu.\n";

  // Mesa
  Malha tampo;
  Vetor3d K_tampo = { 0.6f, 0.3f, 0.1f };
  float m_tampo = 1.0f;
  tampo.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);
  tampo.transformar(
    Matriz::translacao({ 300.0f, 50.0f, 500.0f }) * // Centralize no eixo X e Z
    Matriz::escala({ 250.0f, 5.0f, 150.0f }));

  Vetor3d K_suporte = { 1.0f, 0.5f, 0.5f };
  Vetor3d K_estrela = { 1.0f, 1.0f, 0.0f };
  Vetor3d K_tronco = { 0.65f, 0.33f, 0.16f };
  Vetor3d K_folhas = { 0.0f, 0.39f, 0.0f };
  Malha suporte1, suporte2;
  suporte1.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_suporte, K_suporte, K_suporte, m_tampo);

  suporte2.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_suporte, K_suporte, K_suporte, m_tampo);
  suporte1.transformar(Matriz::translacao({ 425.0f, 2.0f, 500.0f }) *
                       Matriz::escala({ 5.0f, 95.0f, 150.0f }));
  suporte2.transformar(Matriz::translacao({ 175.0f, 2.0f, 500.0f }) *
                       Matriz::escala({ 5.0f, 95.0f, 150.0f }));

  ObjetoComplexo mesa;
  mesa.adicionar_objeto(tampo);
  mesa.adicionar_objeto(suporte1);
  mesa.adicionar_objeto(suporte2);
  complexObjects.push_back(mesa);

  // Árvore de Natal
  Vetor3d dir_cima = { 0.0f, 1.0f, 0.0f };
  Cilindro tronco({ 300.0f, 55.0f, 500.0f },
                  6.0f,
                  40.0f,
                  dir_cima,
                  K_tronco,
                  K_tronco,
                  K_tronco,
                  10.0f);
  // 300, 55, 300 -> {300, 55, 300, 1}, {0,0,0,0}
  Cone folhas({ 300.0f, 100.0f, 500.0f },
              60.0f,
              150.0f,
              dir_cima,
              K_folhas,
              K_folhas,
              K_folhas,
              m_tampo);
  // 300, 100, 300 -> 1, caso seja ponto, caso seja direcao = 0
  Esfera estrela(
    { 300.0f, 250.0f, 500.0f }, 4.5f, K_estrela, K_estrela, K_estrela, m_tampo);
  Cilindro suporte_arvore({ 300.0f, 55.0f, 500.0f },
                          30.0f,
                          9.0f,
                          dir_cima,
                          K_estrela,
                          K_estrela,
                          K_estrela,
                          10.0f);
  Circulo suporte_arvore_circulo({ 300.0f, 64.0f, 500.0f },
                                 30.0f,
                                 { 0.0f, 1.0f, 0.0f },
                                 K_estrela,
                                 K_estrela,
                                 K_estrela,
                                 10.0f);

  ObjetoComplexo arvore;
  arvore.adicionar_objeto(tronco);
  arvore.adicionar_objeto(folhas);
  arvore.adicionar_objeto(estrela);
  arvore.adicionar_objeto(suporte_arvore);
  arvore.adicionar_objeto(suporte_arvore_circulo);
  complexObjects.push_back(arvore);

  // Pórtico 1
  Malha coluna1, coluna2, viga1, viga2;
  coluna1.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);
  coluna2.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);
  viga1.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);
  viga2.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);

  coluna1.transformar(Matriz::translacao({ 0.0f, 250.0f, 0.0f }) *
                      Matriz::escala({ 50.0f, 500.0f, 30.0f }));
  // 0, 250, 600,1
  coluna2.transformar(Matriz::translacao({ 600.0f, 250.0f, 0.0f }) *
                      Matriz::escala({ 50.0f, 500.0f, 30.0f }));
  viga1.transformar(Matriz::translacao({ 150.0f, 587.5f, 0.0f }) *
                    Matriz::cisalhamento_xy_y(atan(0.75)) *
                    Matriz::escala({ 300.0f, 50.0f, 30.0f }));
  viga2.transformar(Matriz::translacao({ 450.0f, 587.5f, 0.0f }) *
                    Matriz::cisalhamento_xy_y(atan(-0.75)) *
                    Matriz::escala({ 300.0f, 50.0f, 30.0f }));

  ObjetoComplexo portico;
  portico.adicionar_objeto(coluna1);
  portico.adicionar_objeto(coluna2);
  portico.adicionar_objeto(viga1);
  portico.adicionar_objeto(viga2);
  complexObjects.push_back(portico);

  // Pórtico 2
  Malha coluna12, coluna22, viga12, viga22;
  coluna12.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);
  coluna22.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);
  viga12.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);
  viga22.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_tampo, K_tampo, K_tampo, m_tampo);

  coluna12.transformar(Matriz::translacao({ 0.0f, 250.0f, 1001.0f }) *
                       Matriz::escala({ 50.0f, 500.0f, 30.0f }));
  coluna22.transformar(Matriz::translacao({ 600.0f, 250.0f, 1001.0f }) *
                       Matriz::escala({ 50.0f, 500.0f, 30.0f }));
  viga12.transformar(Matriz::translacao({ 150.0f, 587.5f, 1001.0f }) *
                     Matriz::cisalhamento_xy_y(atan(0.75)) *
                     Matriz::escala({ 300.0f, 50.0f, 30.0f }));
  viga22.transformar(Matriz::translacao({ 450.0f, 587.5f, 1001.0f }) *
                     Matriz::cisalhamento_xy_y(atan(-0.75)) *
                     Matriz::escala({ 300.0f, 50.0f, 30.0f }));

  ObjetoComplexo portico2;
  portico2.adicionar_objeto(coluna12);
  portico2.adicionar_objeto(coluna22);
  portico2.adicionar_objeto(viga12);
  portico2.adicionar_objeto(viga22);
  complexObjects.push_back(portico2);

  Vetor3d K_parede = { 0.0f, 0.0f, 1.0f };
  Vetor3d K_telhado = { 1.0f, 0.0f, 0.0f };
  float m_parede = 1.0f;

  // --- TELHADO ESQUERDO ---
  Malha telhado_esq;
  telhado_esq.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_telhado, K_telhado, K_telhado, m_parede);
  telhado_esq.transformar(Matriz::translacao({ 150.0f, 600.0f, 500.0f }) *
                          Matriz::cisalhamento_xy_y(atan(0.75)) *
                          Matriz::escala({ 300.0f, 20.0f, 1000.0f }));

  // --- TELHADO DIREITO ---
  Malha telhado_dir;
  telhado_dir.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_telhado, K_telhado, K_telhado, m_parede);
  telhado_dir.transformar(Matriz::translacao({ 450.0f, 600.0f, 500.0f }) *
                          Matriz::cisalhamento_xy_y(atan(-0.75)) *
                          Matriz::escala({ 300.0f, 20.0f, 1000.0f }));

  // --- PAREDE DE FECHAMENTO ESQUERDA ---
  Malha parede_esq;
  parede_esq.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_parede, K_parede, K_parede, m_parede);
  parede_esq.transformar(Matriz::translacao({ 0.0f, 250.0f, 500.0f }) *
                         Matriz::escala({ 20.0f, 500.0f, 1000.0f }));

  // --- PAREDE DE FECHAMENTO DIREITA ---
  Malha parede_dir;
  parede_dir.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_parede, K_parede, K_parede, m_parede);
  parede_dir.transformar(Matriz::translacao({ 600.0f, 250.0f, 500.0f }) *
                         Matriz::escala({ 20.0f, 500.0f, 1000.0f }));

  // --- PAREDE DE FECHAMENTO DE FUNDO ---
  Malha parede_fundo;
  parede_fundo.inicializar_cubo(
    { 0.0f, 0.0f, 0.0f }, 1.0f, K_parede, K_parede, K_parede, m_parede);
  parede_fundo.transformar(Matriz::translacao({ 300.0f, 250.0f, 0.0f }) *
                           Matriz::escala({ 600.0f, 500.0f, -20.0f }));

  ObjetoComplexo paredes_e_telhados;
  paredes_e_telhados.adicionar_objeto(telhado_esq);
  paredes_e_telhados.adicionar_objeto(telhado_dir);
  paredes_e_telhados.adicionar_objeto(parede_esq);
  paredes_e_telhados.adicionar_objeto(parede_dir);
  paredes_e_telhados.adicionar_objeto(parede_fundo);

  complexObjects.push_back(paredes_e_telhados);
}
void flatten_objetos(ObjetoComplexo& objeto_complexo,
                std::vector<Objeto>& objetos_flat)
{
    std::stack<ObjetoComplexo*> pilha;
    pilha.push(&objeto_complexo);
    while (!pilha.empty()) {
        ObjetoComplexo* atual = pilha.top();
        pilha.pop();
        for (auto& obj : atual->objetos) {
            objetos_flat.push_back(obj);
        }
        for (auto& sub_objeto : atual->objetosComplexos) {
            pilha.push(&sub_objeto);
        }
    }
}

void deletar_objetos(Color *pixels_textura, Image textura)
{
  UnloadImageColors(pixels_textura);
  UnloadImage(textura);
}