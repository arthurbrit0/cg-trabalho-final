#include <cmath>
#include <cstdio>
#include <functional>
#include <iostream>
#include <omp.h>
#include <raylib.h>
#include <raymath.h>
#include <string>
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
#include "src/objetosTrabalhofinal.h"

using namespace funcoes_auxiliares;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 500;
const Color BACKGROUND_COLOR = {44, 44, 44, 255};
const Color BUTTON_COLOR = {54, 54, 54, 255};
const float BUTTON_FONTSIZE = 20.0f;

// definicao das dimensoes da janela
const int W_C = 500;
const int H_C = 500;

// definicao das dimensoes do frame
float W_J = 60.0f;
float H_J = 60.0f;

// definicao do numero de linhas do frame
const int nLin = 500;

// definicao do numero de colunas do frame
const int nCol = 500;

// distancia do frame ao olho
float d = 30.0f;

double deltinhax = W_J / nCol, deltinhay = H_J / nLin;
int Deltax = W_C / nCol, Deltay = H_C / nLin;
Vetor3d Ponto_Superior_Esquerdo = {-W_J * 0.5f, W_J * 0.5f, -d};
float zp = -d;

// definicao da fonte luminosa
std::vector<iluminacao::FontePontual> fontes_pontuais;
std::vector<iluminacao::FonteDirecional> fontes_direcionais;
std::vector<iluminacao::FonteSpot> fontes_spot;

std::vector<std::string> fontes_pontuais_labels;
std::vector<std::string> fontes_direcionais_labels;
std::vector<std::string> fontes_spot_labels;

// definicao da iluminacao ambiente
Vetor3d I_A = {0.4f, 0.4f, 0.4f};

std::vector<ObjetoComplexo> complexObjects;
std::vector<Objeto> objetos;

RenderTexture2D tela;
bool ortografica = false;
// Inicializar câmera
Vetor3d Eye = {0.0f, 400.0f, 840.0f}; // Elevada, para ver o chão e os objetos sobre ele
Vetor3d At = {0.0f, 0.0f, 0.0f};      // Olha para o centro da cena
Vetor3d Up = {0.0f, 1.0f, 0.0f};
Camera3de camera(Eye, At, Up);

std::vector<Color>
pixel_buffer(nLin *nCol, WHITE);

void renderizar()
{
  TraceLog(LOG_INFO, "Renderizando");

  for (int i = 0; i < pixel_buffer.size(); ++i)
  {
    pixel_buffer[i] = WHITE;
  }

  Matriz M_cw = camera.getMatrixCameraWorld();

  Vetor3d PSE = (M_cw * camera.get_PSE().ponto4d()).vetor3d();
  Vetor3d right = {1.0f, 0.0f, 0.0f};
  Vetor3d down = {0.0f, -1.0f, 0.0f};
  Vetor3d forward = camera.get_center().normalizado();
  right = (M_cw * right.vetor4d()).vetor3d();
  down = (M_cw * down.vetor4d()).vetor3d();
  forward = (M_cw * forward.vetor4d()).vetor3d();

  deltinhax = camera.get_W_J() / nCol, deltinhay = camera.get_H_J() / nLin;

  BeginTextureMode(tela);
  {
    ClearBackground(BLACK);

    for (int i = 0; i < nLin; ++i)
    {
      for (int j = 0; j < nCol; ++j)
      {
        Vetor3d P = PSE + right * (deltinhax * (j + 0.5f)) +
                    down * (deltinhay * (i + 0.5f));
        Vetor3d dr;
        if (ortografica)
        {
          dr = forward;
        }
        else
        {
          dr = (P - camera.position).normalizado();
        }
        Raio raio(ortografica ? P : camera.position, dr);
        auto [t, objeto] = calcular_intersecao(raio, objetos);
        if (t > 0.0f)
        {
          Vetor3d I_total = {0.0f, 0.0f, 0.0f};
          Vetor3d Pt = raio.no_ponto(t);
          Vetor3d normal = objetos[objeto].normal(Pt);
          MaterialSimples material;

          std::visit(
              [&](auto &&obj)
              {
                using T = std::decay_t<decltype(obj)>;
                if constexpr (std::is_same_v<T, PlanoTextura>)
                {
                  material = obj.material(Pt);
                }
                else
                {
                  material = objetos[objeto].material;
                }
              },
              objetos[objeto].obj);

          for (const iluminacao::FontePontual &fonte : fontes_pontuais)
          {
            if (!fonte.acesa)
              continue;
            Vetor3d dr_luz = fonte.posicao - Pt;
            float dist_luz = dr_luz.tamanho();
            if (dist_luz == 0.0f)
              continue;
            dr_luz = dr_luz * (1.0f / dist_luz);
            Raio raio_luz(Pt, dr_luz);
            auto [t_luz, _] = calcular_intersecao(raio_luz, objetos, objeto);
            if (t_luz < 0.0 || t_luz > dist_luz)
            {
              I_total = I_total + modelo_phong(
                                      Pt, raio.dr, normal, fonte, material);
            }
          }
          for (iluminacao::FonteDirecional &fonte : fontes_direcionais)
          {
            if (!fonte.acesa)
              continue;
            Vetor3d dr_luz = fonte.direcao.normalizado();
            Raio raio_luz(Pt, dr_luz);
            auto [t_luz, _] = calcular_intersecao(raio_luz, objetos, objeto);
            if (t_luz < 0.0)
            {

              Vetor3d cor_direcional =
                  modelo_phong(Pt, raio.dr, normal, fonte, material);
              I_total =
                  I_total + modelo_phong(Pt, raio.dr, normal, fonte, material);
            }
          }
          for (const iluminacao::FonteSpot &fonte : fontes_spot)
          {
            if (!fonte.acesa)
              continue;
            Vetor3d dr_luz = fonte.posicao - Pt;
            float dist_luz = dr_luz.tamanho();
            dr_luz = dr_luz * (1.0f / dist_luz);
            Raio raio_luz(Pt, dr_luz);
            auto [t_luz, _] = calcular_intersecao(raio_luz, objetos, objeto);
            if (t_luz < 0.0 || t_luz > dist_luz)
            {
              I_total = I_total + modelo_phong(
                                      Pt, raio.dr, normal, fonte, material);
            }
          }

          I_total = I_total + iluminacao::luz_ambiente(I_A, material.K_a);

          pixel_buffer[i * nCol + j] = {
              static_cast<unsigned char>(fmin(I_total.x * 255.0f, 255.0f)),
              static_cast<unsigned char>(fmin(I_total.y * 255.0f, 255.0f)),
              static_cast<unsigned char>(fmin(I_total.z * 255.0f, 255.0f)),
              255};
        }
      }
    }

    for (int i = 0; i < nLin; ++i)
    {
      for (int j = 0; j < nCol; ++j)
      {
        DrawPixel(j, i, pixel_buffer[i * nCol + j]);
      }
    }
  }
  EndTextureMode();
  TraceLog(LOG_INFO, "Renderizacao completa");
}

void inicializar_luzes()
{
  fontes_pontuais.push_back(
      iluminacao::FontePontual({600.0f, 200.0f, 1500.0f}, {0.8f, 0.8f, 0.8f}));
  fontes_pontuais_labels.push_back("luz_pontual");

  fontes_direcionais.push_back(
      iluminacao::FonteDirecional({-1.0f, 1.0f, -1.0f}, {0.5f, 0.5f, 0.5f}));
  fontes_direcionais_labels.push_back("luz_direcional");

  fontes_spot.push_back(iluminacao::FonteSpot({800.0f, 600.0f, 200.0f},
                                              {1.0f, 1.0f, 1.0f},
                                              PI / 6,
                                              {0.5f, 0.8f, 0.8f}));
  fontes_spot_labels.push_back("luz_spot");
}

int main()
{
  // Inicializações básicas
  omp_set_num_threads(8);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trabalho Final");
  SetTargetFPS(60);

  Font font = GetFontDefault();
  int objeto_selecionado = -1; // índice do objeto selecionado

  // Carrega a renderização para a cena (lado esquerdo)
  tela = LoadRenderTexture(W_C, H_C);

  // Inicializa os objetos e a iluminação
  inicializar_objetosfinal(objetos, complexObjects);
  for (ObjetoComplexo &objeto_complexo : complexObjects)
  {
    flatten_objetos(objeto_complexo, objetos);
  }
  std::cout << "Objetos na cena: " << objetos.size() << "\n";

  inicializar_luzes();

  // Atualiza a câmera para garantir que os parâmetros estejam corretos
  camera.updateCoordinates();

  // Renderiza inicialmente a cena
  renderizar();

  // Loop principal
  while (!WindowShouldClose())
  {
    // Processa o zoom antes do tratamento do mouse e do desenho
    if (IsKeyPressed(KEY_UP))
    {
      if (ortografica)
      {
        Vetor3d centro = camera.get_center();
        float dx = (camera.xmax - centro.x) * 0.10f;
        float dy = (camera.ymax - centro.y) * 0.10f;
        camera.xmax -= dx;
        camera.xmin += dx;
        camera.ymax -= dy;
        camera.ymin += dy;
      }
      else
      {
        // Zoom in: move a câmera mais perto do lookAt
        Vetor3d dv = (camera.lookAt - camera.position) * 0.10f;
        camera.position = camera.position + dv;
        camera.lookAt = camera.lookAt + dv;
      }
      camera.updateCoordinates();
      renderizar();
    }
    if (IsKeyPressed(KEY_DOWN))
    {
      if (ortografica)
      {
        Vetor3d centro = camera.get_center();
        float dx = (camera.xmax - centro.x) * 0.10f;
        float dy = (camera.ymax - centro.y) * 0.10f;
        camera.xmax += dx;
        camera.xmin -= dx;
        camera.ymax += dy;
        camera.ymin -= dy;
      }
      else
      {
        // Zoom out: afasta a câmera do lookAt
        Vetor3d dv = (camera.lookAt - camera.position) * 0.10f;
        camera.position = camera.position - dv;
        camera.lookAt = camera.lookAt - dv;
      }
      camera.updateCoordinates();
      renderizar();
    }

    // Atualização dos parâmetros da câmera para a renderização
    Matriz M_cw = camera.getMatrixCameraWorld();
    Vetor3d PSE = (M_cw * camera.get_PSE().ponto4d()).vetor3d();
    Vetor3d right = {1.0f, 0.0f, 0.0f};
    Vetor3d down = {0.0f, -1.0f, 0.0f};
    Vetor3d forward = camera.get_center().normalizado();
    right = (M_cw * right.vetor4d()).vetor3d();
    down = (M_cw * down.vetor4d()).vetor3d();
    forward = (M_cw * forward.vetor4d()).vetor3d();
    deltinhax = camera.get_W_J() / nCol;
    deltinhay = camera.get_H_J() / nLin;

    Vector2 mouse = GetMousePosition();

    // Processa seleção de objeto na área de renderização (lado esquerdo)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse.x < W_C && mouse.y < H_C)
    {
      int x_pos = mouse.x, y_pos = mouse.y;
      Vetor3d P = PSE + right * (deltinhax * (x_pos + 0.5f)) +
                  down * (deltinhay * (y_pos + 0.5f));
      Vetor3d dr = ortografica ? forward : (P - camera.position).normalizado();
      Raio raio(ortografica ? P : camera.position, dr);
      auto [t, objeto] = calcular_intersecao(raio, objetos);
      if (t > 0.0f)
      {
        objeto_selecionado = objeto;
        TraceLog(LOG_INFO, "Objeto selecionado: %d", objeto_selecionado);
      }
    }
    // Processa clique na área de interface para alternar o "Visível"
    else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mouse.x >= W_C)
    {
      Rectangle buttonRect = {520.0f, 20.0f, 100.0f, 30.0f};
      if (CheckCollisionPointRec(mouse, buttonRect) && objeto_selecionado >= 0)
      {
        objetos[objeto_selecionado].visivel = !objetos[objeto_selecionado].visivel;
        TraceLog(LOG_INFO, "Objeto %d visivel: %d", objeto_selecionado, objetos[objeto_selecionado].visivel);
        renderizar();
      }
    }

    BeginDrawing();
    {
      ClearBackground(BACKGROUND_COLOR);
      if (objeto_selecionado >= 0)
      {
        Rectangle buttonRect = {520.0f, 20.0f, 100.0f, 30.0f};
        DrawRectangleRec(buttonRect, BUTTON_COLOR);
        std::string label = "Visivel: ";
        label += (objetos[objeto_selecionado].visivel ? "ON" : "OFF");
        Vector2 textSize = MeasureTextEx(font, label.c_str(), 20.0f, 3.0f);
        Vector2 textPos = {buttonRect.x + (buttonRect.width - textSize.x) / 2,
                           buttonRect.y + (buttonRect.height - textSize.y) / 2};
        DrawTextEx(font, label.c_str(), textPos, 20.0f, 3.0f, WHITE);
      }
      DrawTextureRec(tela.texture, {0.0f, 0.0f, (float)W_C, -(float)H_C}, {0.0f, 0.0f}, WHITE);
    }
    EndDrawing();
  }

  CloseWindow();
  UnloadRenderTexture(tela);
  return 0;
}
