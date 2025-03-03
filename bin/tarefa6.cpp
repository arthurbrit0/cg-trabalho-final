#include <cmath>
#include <cstdio>
#include <iostream>
#include <omp.h>
#include <raylib.h>
#include <vector>

#include "./src/Camera/Camera3de.h"
#include "./src/Iluminacao/Iluminacao.h"
#include "./src/Material/Material.h"
#include "./src/Objeto/Objeto.h"
#include "./src/Raio/Raio.h"
#include "funcoes_auxiliares.h"
#include "src/ObjetoComplexo/ObjetoComplexo.h"
#include "src/calcular_intersecao.h"
#include "src/inicializar_objetos.h"

using namespace funcoes_auxiliares;

// Dimensões da janela e frame
const int W_C = 500;
const int H_C = 500;
const float W_J = 60.0f;
const float H_J = 60.0f;
const int nLin = 500, nCol = 500;
const float d = 50.0f;

// Illumination e fonts de luz
Vetor3d I_A = { 0.7f, 0.7f, 0.7f };
Vetor3d P_F = { 300.0f, 300.0f, 1500.0f };
Vetor3d I_F = { 1.0f, 1.0f, 1.0f };

std::vector<ObjetoComplexo> complexObjects;
std::vector<Objeto> objects_flat;

int
main()
{
  omp_set_num_threads(16);
  InitWindow(W_C, H_C, "Tarefa06 ");
  SetTargetFPS(60);

  // Inicializar câmera
  Vetor3d Eye = { 150.0f, 125.0f, 800.0f };
  Vetor3d At = { 300.0f, 125.0f, 500.0f };
  Vetor3d Up = { 300.0f, 325.0f, 500.0f };
  Camera3de camera(Eye, At, Up);
  Matriz M_wc = camera.getTransformationMatrix();

  P_F = (M_wc * P_F.ponto4d()).vetor3d();

  inicializar_objetos(objects_flat, complexObjects);

  for (ObjetoComplexo& objeto_complexo : complexObjects) {
    flatten_objetos(objeto_complexo, objects_flat);
  }
  // Cada objeto irá transformar em 4x4
  for (auto& objeto : objects_flat) {

    objeto.transformar(M_wc);
  }
  std::cout << "Objetos na cena: " << objects_flat.size() << "\n";
  // Loop principal
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    double deltinhax = W_J / nCol, deltinhay = H_J / nLin;
    Vetor3d Ponto_Superior_Esquerdo = { -W_J * 0.5f, W_J * 0.5f, -d };

    std::vector pixel_buffer(nLin * nCol, WHITE);

#pragma omp parallel for
    for (int i = 0; i < nLin; ++i) {
      for (int j = 0; j < nCol; ++j) {
        float yp = Ponto_Superior_Esquerdo.y - deltinhay * 0.5f - i * deltinhay;
        float xp = Ponto_Superior_Esquerdo.x + deltinhax * j + 0.5f * deltinhax;
        Vetor3d P = { xp, yp, -d };
        Vetor3d dr = P.normalizado();
        Raio raio(camera.position, dr);

        auto [t, object] = calcular_intersecao(raio, objects_flat);
        Vetor3d I_total = { 0.0f, 0.0f, 0.0f };
        MaterialSimples material;

        if (t > 0.0f) {
          Vetor3d Pt = raio.no_ponto(t);
          Vetor3d normal = objects_flat[object].normal(Pt);

          std::visit(
            [&](auto&& obj) {
              using T = std::decay_t<decltype(obj)>;
              if constexpr (std::is_same_v<T, PlanoTextura>) {
                material = obj.material(Pt);
              } else {
                material = objects_flat[object].material;
              }
            },
            objects_flat[object].obj);

          Vetor3d dr_luz = (P_F - Pt).normalizado();
          Raio light_ray(Pt, dr_luz);

          if (auto [t_luz, _] =
                calcular_intersecao(light_ray, objects_flat, object);
              t_luz < 0.0f || t_luz > (P_F - Pt).tamanho()) {
            I_total = I_total + iluminacao::modelo_phong(
                                  Pt,
                                  raio.dr,
                                  normal,
                                  iluminacao::FontePontual(P_F, I_F),
                                  material);
          }
          I_total = I_total + iluminacao::luz_ambiente(I_A, material.K_a);

          pixel_buffer[i * nCol + j] = {
            static_cast<unsigned char>(fmin(I_total.x * 255, 255)),
            static_cast<unsigned char>(fmin(I_total.y * 255, 255)),
            static_cast<unsigned char>(fmin(I_total.z * 255, 255)),
            255
          };
        } else {
          pixel_buffer[i * nCol + j] = WHITE;
        }
      }
    }

    // Depois que os threads terminam de trabalhar, o código desenha os pixels
    for (int i = 0; i < nLin; ++i) {
      for (int j = 0; j < nCol; ++j) {
        DrawPixel(j, i, pixel_buffer[i * nCol + j]);
      }
    }

    EndDrawing();
  }
  CloseWindow();

  return 0;
}