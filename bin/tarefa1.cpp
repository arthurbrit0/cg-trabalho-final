#include <math.h>
#include <raylib.h>
#include "funcoes_auxiliares.h"

using namespace funcoes_auxiliares;

// Definição das dimensões da janela
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Definição das dimensões do frame
const float wJanela = 8.0;
const float hJanela = 6.0;

// Definição do número de linhas do frame
const int nLin = 600;

// Definição do número de colunas do frame
const int nCol = 800;

// Distância do frame ao olho
float dJanela = 2.0;

// Posição do olho do observador
Vetor3d E = {0.0f, 0.0f, 0.0f};

// Raio da esfera
float rEsfera = 6.0;

// Centro da esfera
Vetor3d centro_esfera = { 0.0, 0.0, -(dJanela + rEsfera) };

// Cor da esfera
Vetor3d esfColor = {255.0, 0.0, 0.0};

// Cor do background
Vetor3d bgColor = {100.0, 100.0, 100.0};

int main(void) {
    // Inicialização da janela
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tarefa 01");
    SetTargetFPS(60);

    // Inicialização do dx e do dy
    double deltinhax = wJanela / nCol, deltinhay = hJanela / nLin;

    // Determinação de quantos pixels correspondem a cada quadrado do frame
    int Deltax = SCREEN_WIDTH / nCol, Deltay = SCREEN_HEIGHT / nLin;

    // Loop de renderização
    while (!WindowShouldClose()) {
        // Definição do ponto superior esquerdo do frame
        Vetor3d PSE = { -wJanela * 0.5f, hJanela * 0.5f, -dJanela };
        // Definição da coordenada z dos pontos do frame
        float zp = -dJanela;
        // Definição do vetor v que vai do centro da esfera ao olho do observador
        Vetor3d v = E - centro_esfera;

        // Início do código de desenho
        BeginDrawing(); {
            ClearBackground(BLACK);

            // Loop sobre as linhas
            for (int i = 0; i < nLin; ++i) {
                // Cálculo do y do centro dos pontos da linha i
                float yp = PSE.y - deltinhay * 0.5 - i * deltinhay;
                // Loop sobre as colunas
                for (int j = 0; j < nCol; ++j) {
                    // Cálculo do x do centro do ponto da coluna j
                    float xp = PSE.x + deltinhax * 0.5 + j * deltinhax;
                    // Construção do ponto P que representa o centro do quadrado i, j do frame
                    Vetor3d P = { xp, yp, zp };
                    // Vetor dr normalizado que aponta do olho do observador ao ponto P
                    Vetor3d dr = (P - E).normalizado();

                    // Cálculo dos coeficientes da equação de interseção entre a esfera e o raio
                    double a = dr.dot_product(dr);
                    double b = dr.dot_product(v);
                    double c = v.dot_product(v) - rEsfera * rEsfera;

                    // Delta da equação
                    double delta = b * b - a * c;
                    // Se o delta for menor que zero, não há interseção
                    if(delta < 0.0) {
                        DrawRectangle(Deltax * j, Deltax * i, Deltax, Deltay, (Color) { (unsigned char)bgColor.x, (unsigned char)bgColor.y, (unsigned char)bgColor.z, 255 });
                        continue;
                    }

                    // Como a é sempre não negativo e a raiz quadrada também
                    // esse é o menor valor de t
                    double t = (-b - sqrt(delta)) / a;
                    // Se esse t é negativo, tentamos o outro
                    if(t < 0.0) {
                        t = (-b + sqrt(delta)) / a;
                    }
                    // Se esse t também é negativo a interseção ocorreu
                    // atrás do campo de visão, logo não é contabilizada
                    if (t < 0.0) {
                        DrawRectangle(Deltax * j, Deltax * i, Deltax, Deltay, (Color) { (unsigned char)bgColor.x, (unsigned char)bgColor.y, (unsigned char)bgColor.z, 255 });
                        continue;
                    }

                    // Caso chegamos a esse ponto, houve interseção
                    // Logo, colorimos o pixel com a cor da esfera
                    DrawRectangle(Deltax * j, Deltax * i, Deltax, Deltay, (Color) { (unsigned char)esfColor.x, (unsigned char)esfColor.y, (unsigned char)esfColor.z, 255 });
                }
            }

        }
        EndDrawing();
    }

    return 0;
}
