#include "funcoes_auxiliares.h"
#include <math.h>
#include <raylib.h>
using namespace funcoes_auxiliares;
// definicao das dimensoes da janela
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// definicao das dimensoes do frame
const float width = 8.0;
const float height = 6.0;

// definicao do numero de linhas do frame
const int lin = 600;

// definicao do numero de colunas do frame
const int col = 800;

// distancia do frame ao olho
float dist_ao_quadro = 2.0;

// Posição do olho do observador
Vetor3d E = { 0.0f, 0.0f, 0.0f };

// raio da esfera
double raio = 6.0;
// centro da esfera
Vetor3d centro_esfera = { 0.0, 0.0, -10.0 };

// vetor representando o material da esfera
const Vetor3d K = { 1, 0.0, 0.0 };

// numero representando o expoente do calculo
// da iluminacao especular
const double alpha = 50.0;

// dados da fonte luminosa pontual
Vetor3d I_F = { 0.7, 0.7, 0.7 };
Vetor3d P_F = { 0.0, 5.0, 0.0 };

// vetor representando a luz ambiente
const Vetor3d I_A = { 0.1, 0.1, 0.1 };

int
main(void)
{

  // inicializacao da janela
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raytracing");
  SetTargetFPS(60);

  // inicializacao do dx e do dy
  double deltinhax = width / col, deltinhay = height / lin;

  // determinacao de quantos pixels correspondem a cada quadrado do frame
  int Deltax = SCREEN_WIDTH / col, Deltay = SCREEN_HEIGHT / lin;

  // loop de renderizacao
  while (!WindowShouldClose()) {

    // codigo para mover a esfera
    // e para aproximar ou afastar o frame do observador
    if (IsKeyDown(KEY_W))
      dist_ao_quadro += 0.05;
    if (IsKeyDown(KEY_S) && dist_ao_quadro > 0.10)
      dist_ao_quadro -= 0.05;
    if (IsKeyDown(KEY_J))
      centro_esfera.y += 0.05;
    if (IsKeyDown(KEY_K))
      centro_esfera.y -= 0.05;
    if (IsKeyDown(KEY_H))
      centro_esfera.x -= 0.05;
    if (IsKeyDown(KEY_L))
      centro_esfera.x += 0.05;

    // definicao do ponto superior esquerdo do frame
    Vetor3d Ponto_Superior_Esquerdo = { -width * 0.5,
                                        height * 0.5,
                                        -dist_ao_quadro };
    // definicao da coordenada z dos pontos do frame
    float zp = -dist_ao_quadro;
    // definicao do vetor v que vai do centro da esfera ao olho do observador
    Vetor3d v = E - centro_esfera;

    // inicio do codigo de desenho
    BeginDrawing();
    ClearBackground((Color){ 100, 100, 100, 255 });

    // loop sobre as linhas
    for (int i = 0; i < lin; ++i) {
      // calculo do y do centro dos pontos da linha i
      float yp = Ponto_Superior_Esquerdo.y - deltinhay * 0.5 - i * deltinhay;
      // loop sobre as colunas
      for (int j = 0; j < col; ++j) {
        // calculo do x do centro do ponto da coluna j
        float xp = Ponto_Superior_Esquerdo.x + deltinhax * j + 0.5 * deltinhax;
        // construcao do ponto P que representa o centro do quadrado i, j do
        // frame
        Vetor3d P = { xp, yp, zp };

        // vetor dr normalizado que aponta do olho do observador
        // ao ponto P
        Vetor3d dr = P.normalizado();

        // calculo dos coeficientes da equacao de intersecao entre a esfera e o
        // raio
        double a = dr.dot_product(dr);
        double b = 2 * dr.dot_product(v);
        double c = v.dot_product(v) - raio * raio;

        // delta da equacao
        double deltaQuadratico = b * b - 4 * a * c;

        // se o delta for menor que zero, nao ha intersecao
        if (deltaQuadratico < 0.0) {
          continue;
        }

        // como a e sempre nao negativo e a raiz qudrada tambem
        // esse e o menor valor de t
        double t = (-b - sqrt(deltaQuadratico)) / (2 * a);
        // se esse t e negativo, tentamos o outro
        if (t < 0.0) {
          t = (-b + sqrt(deltaQuadratico)) / (2 * a);
        }
        // se esse t tambem e negativo a intersecao ocorreu
        // atras do campo de visao, logo nao e contabilizada
        if (t < 0.0) {
          continue;
        }

        // calculo do ponto de intersecao a partir de t
        Vetor3d Pt = dr * t;
        // calculo da normal a esfera no ponto de intersecao
        Vetor3d normal = (Pt - centro_esfera).normalizado();
        // vetor na direcao inversa de dt
        Vetor3d v_inverso = dr * -1;
        // vetor do ponto de intersecao a fonte de luz
        Vetor3d vetor_luminoso = (P_F - Pt).normalizado();
        // vetor representando a direcao do raio refletido
        Vetor3d r =
          2 * normal.dot_product(vetor_luminoso) * normal - vetor_luminoso;

        // calculo da intensidade da luz ambiente
        Vetor3d I_ambiente = K * I_A;

        // calculo da intensidade da luz difusa
        Vetor3d I_difusa = K * I_F * max(vetor_luminoso.dot_product(normal), 0);

        // calculo da intensidade da luz especular
        Vetor3d I_especular =
          K * I_F * pow(max(v_inverso.dot_product(r), 0), alpha);

        // calculo da intensidade da luz total
        Vetor3d I_total = I_ambiente + I_difusa + I_especular;

        // desenho do pixel na tela
        DrawRectangle(Deltax * j,
                      Deltay * i,
                      Deltax,
                      Deltay,
                      (Color){ (unsigned char)min(I_total.x * 255.0, 255.0),
                               (unsigned char)min(I_total.y * 255.0, 255.0),
                               (unsigned char)min(I_total.z * 255.0, 255.0),
                               255 });
      }
    }

    EndDrawing();
  }

  return 0;
}
