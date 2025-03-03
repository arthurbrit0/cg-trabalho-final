#include <cstdio>
#include <math.h>
#include <raylib.h>

#include <math.h>
#include <cstdio>
#include <raylib.h>
#include <vector>

#include "funcoes_auxiliares.h"
#include "./src/Circulo/Circulo.h"
#include "./src/Cilindro/Cilindro.h"
#include "./src/Cone/Cone.h"
#include "./src/Esfera/Esfera.h"
#include "./src/Iluminacao/Iluminacao.h"
#include "./src/Material/Material.h"
#include "./src/Plano/Plano.h"
#include "./src/Raio/Raio.h"
#include "./src/Triangulo/Triangulo.h"
#include "./src/Objeto/Objeto.h"
using namespace funcoes_auxiliares;


// definicao das dimensoes da janela
const int W_C = 500;
const int H_C = 500;

// definicao das dimensoes do frame
const float W_J = 60.0f;
const float H_J = 60.0f;

// definicao do numero de linhas do frame
const int nLin = 500;

// definicao do numero de colunas do frame
const int nCol = 500;

// distancia do frame ao olho
float d = 30.0f;

// definicao da esfera da cena
Vetor3d K_d_esfera = { 0.7, 0.2, 0.2 };
Vetor3d K_e_esfera = { 0.7, 0.2, 0.2 };
Vetor3d K_a_esfera = { 0.7, 0.2, 0.2 };
float R = 40.0f;
float m_esfera = 10;
Esfera esfera({ 0.0f, 0.0f, -100.0f },
              R,
              K_d_esfera,
              K_e_esfera,
              K_a_esfera,
              m_esfera);

// definicao do plano do chao
Vetor3d K_d_plano_chao = { 0.2f, 0.7f, 0.2f };
Vetor3d K_e_plano_chao = { 0.0f, 0.0f, 0.0f };
Vetor3d K_a_plano_chao = { 0.2f, 0.7f, 0.2f };
float m_plano_chao = 1;
Plano plano_chao({ 0.0f, -R, 0.0f },
                 { 0.0f, 1.0f, 0.0f },
                 K_d_plano_chao,
                 K_e_plano_chao,
                 K_a_plano_chao,
                 m_plano_chao);

// definicao do plano de fundo
Vetor3d K_d_plano_fundo = { 0.3f, 0.3f, 0.7f };
Vetor3d K_e_plano_fundo = { 0.0f, 0.0f, 0.0f };
Vetor3d K_a_plano_fundo = { 0.3f, 0.3f, 0.7f };
float m_plano_fundo = 1;
Plano plano_fundo({ 0.0f, 0.0f, -200.0f },
                  { 0.0f, 0.0f, 1.0f },
                  K_d_plano_fundo,
                  K_e_plano_fundo,
                  K_a_plano_fundo,
                  m_plano_fundo);

// definicao da fonte luminosa
Vetor3d I_F = { 0.7f, 0.7f, 0.7f };
Vetor3d P_F = { 0.0f, 60.0f, -30.0f };

// definicao da iluminacao ambiente
Vetor3d I_A = { 0.3f, 0.3f, 0.3f };

int
main(void)
{
  // inicializacao da janela
  InitWindow(W_C, H_C, "Tarefa 03");
  SetTargetFPS(60);

  // inicializacao do dx e do dy
  double deltinhax = W_J / nCol, deltinhay = H_J / nLin;

  // determinacao de quantos pixels correspondem a cada quadrado do frame
  int Deltax = W_C / nCol, Deltay = H_C / nLin;

  // definicao do ponto superior esquerdo do frame
  Vetor3d Ponto_Superior_Esquerdo = { -W_J * 0.5f, W_J * 0.5f, -d };

  // definicao da coordenada z dos pontos do frame
  float zp = -d;

  // definicao da posição do olho do observador
  Vetor3d P0 = { 0.0f, 0.0f, 0.0f };

  // loop de renderizacao
  while (!WindowShouldClose()) {

    // inicio do codigo de desenho
    BeginDrawing();
    {
      ClearBackground(BLACK);

      // loop sobre as linhas
      for (int i = 0; i < nLin; ++i) {
        // calculo do y do centro dos pontos da linha i
        float yp = Ponto_Superior_Esquerdo.y - deltinhay * 0.5 - i * deltinhay;
        // loop sobre as colunas
        for (int j = 0; j < nCol; ++j) {
          // calculo do x do centro do ponto da coluna j
          float xp =
            Ponto_Superior_Esquerdo.x + deltinhax * j + 0.5 * deltinhax;
          // construcao do ponto P que representa o centro do quadrado i, j do
          // frame
          Vetor3d P = { xp, yp, zp };

          // vetor dr normalizado que aponta do olho do observador
          // ao ponto P
          Vetor3d dr = (P - P0).normalizado();
          // Criação do raio que parte do olho do observador
          Raio raio(P0, dr);
          // cálculo do t de interseção da esfera
          float t_esfera = raio.intersecao(esfera);
          // cálculo do t de interseção do plano do chão
          float t_plano_chao = raio.intersecao(plano_chao);
          // cálculo do t de interseção do plano de fundo
          float t_plano_fundo = raio.intersecao(plano_fundo);
          Vetor3d I_total = I_A;
          // Efeito de iluminação da Esfera
          if (t_esfera > 0.0f &&
              (t_esfera < t_plano_chao || t_plano_chao < 0.0f) &&
              (t_esfera < t_plano_fundo || t_plano_fundo < 0.0f)) {
            Vetor3d Pt = raio.no_ponto(t_esfera);
            I_total = esfera.calcular_iluminacao(Pt, raio.dr, P_F, I_F, I_A);
          }
          // Efeito de iluminação do chão
          else if (t_plano_chao > 0.0f &&
                   (t_plano_chao < t_esfera || t_esfera < 0.0f) &&
                   (t_plano_chao < t_plano_fundo || t_plano_fundo < 0.0f)) {
            Vetor3d Pt = raio.no_ponto(t_plano_chao);
            Vetor3d dr_luz = (P_F - Pt).normalizado();
            Raio raio_luz(Pt, dr_luz);
            // Se o raio da luz não intercepta a esfera
            // calculamos a contribuição da fonte pontual
            if (raio_luz.intersecao(esfera) < 0.0) {
              I_total =
                plano_chao.calcular_iluminacao(Pt, raio.dr, P_F, I_F, I_A);
            }
            // Do contrário, não calculamos
            else {
              I_total = plano_chao.calcular_iluminacao(
                Pt, raio.dr, P_F, { 0.0f, 0.0f, 0.0f }, I_A);
            }

          }
          // Efeito de iluminação do plano de fundo
          else if (t_plano_fundo > 0.0f &&
                   (t_plano_fundo < t_esfera || t_esfera < 0.0f) &&
                   (t_plano_fundo < t_plano_chao || t_plano_chao < 0.0f)) {
            Vetor3d Pt = raio.no_ponto(t_plano_fundo);
            Vetor3d dr_luz = (P_F - Pt).normalizado();
            Raio raio_luz(Pt, dr_luz);
            // Se o raio da luz não intercepta a esfera
            // calculamos a contribuição da fonte pontual
            if (raio_luz.intersecao(esfera) < 0.0) {
              I_total =
                plano_fundo.calcular_iluminacao(Pt, raio.dr, P_F, I_F, I_A);
            }
            // Do contrário, não calculamos
            else {
              I_total = plano_fundo.calcular_iluminacao(
                Pt, raio.dr, P_F, { 0.0f, 0.0f, 0.0f }, I_A);
            }
          }

          // desenho do pixel na tela
          DrawRectangle(
            Deltax * j,
            Deltay * i,
            Deltax,
            Deltay,
            (Color){
              static_cast<unsigned char>(min(I_total.x * 255.0f, 255.0f)),
              static_cast<unsigned char>(min(I_total.y * 255.0f, 255.0f)),
              static_cast<unsigned char>(min(I_total.z * 255.0f, 255.0f)),
              255 });
        }
      }
    }
    EndDrawing();
  }

  return 0;
}
