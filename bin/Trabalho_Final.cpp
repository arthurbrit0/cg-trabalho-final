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
const Color TEXTBOX_COLOR = {64, 64, 64, 255};
const Color BUTTON_COLOR = {54, 54, 54, 255};
const Color SWITCH_ON_COLOR = {54, 200, 54, 255};
const Color SWITCH_OFF_COLOR = {64, 64, 64, 255};
const float BUTTON_FONTSIZE = 20.0f;
const float TEXTBOX_PADDING = 2.0f;
const float SWITCH_LABEL_MARGIN = 5.0f;
const float LABEL_MARGIN = 4.0f;
const float ELEMENT_MARGIN = 6.0f;
const float TAB_LABEL_HEIGHT = 15.0f;
const float TAB_LABEL_PADDING = 2.0f;
const float TAB_LABEL_MARGIN_LEFT = 5.0f;
const float SCROLL_AMOUNT = 10.0f;
const float TAB_SCROLL_AMOUNT = 5.0f;
const Color TAB_UNSELECTED_COLOR = {84, 84, 84, 255};

struct TextBox
{
  Rectangle rect;
  std::string label;
  char texto[30] = {0};
  size_t capacidade = sizeof(texto) / sizeof(texto[0]) - 1;
  size_t cursor = 0;
  float *parametro = nullptr;

  TextBox(std::string label,
          int x,
          int y,
          int width,
          int height,
          float *parametro)
      : label(label), parametro(parametro)
  {
    rect = (Rectangle){(float)x, (float)y, (float)width, (float)height};
  }

  TextBox(std::string label, Rectangle rect, float *parametro)
      : label(label), rect(rect), parametro(parametro)
  {
  }

  void desenhar(Font font)
  {
    float font_size = rect.height - 2 * TEXTBOX_PADDING;
    float label_height = rect.height;
    Vector2 label_pos = {rect.x, rect.y};
    DrawTextEx(font, label.c_str(), label_pos, font_size, 3.0f, WHITE);
    Rectangle box_rect = {
        rect.x, rect.y + label_height + LABEL_MARGIN, rect.width, rect.height};
    DrawRectangleRec(box_rect, TEXTBOX_COLOR);
    Vector2 text_pos = {box_rect.x + TEXTBOX_PADDING,
                        box_rect.y + TEXTBOX_PADDING};
    DrawTextEx(font, texto, text_pos, font_size, 3.0f, WHITE);
  }

  bool intersecao(Vector2 ponto)
  {
    float label_height = rect.height;
    Rectangle box_rect = {
        rect.x, rect.y + label_height + LABEL_MARGIN, rect.width, rect.height};
    return CheckCollisionPointRec(ponto, box_rect);
  }

  void move(const Vector2 &pos)
  {
    rect.x = pos.x;
    rect.y = pos.y;
  }

  void atualizar(int key, char key_char)
  {
    if (key == KEY_BACKSPACE)
    {
      if (cursor > 0)
      {
        texto[--cursor] = '\0';
      }
    }
    else if (key != 0 && cursor < capacidade && key != KEY_LEFT_SHIFT)
    {
      texto[cursor++] = key_char;
    }
  }

  void atualizar_parametro() { sscanf(texto, "%f", parametro); }

  void atualizar_texto()
  {
    cursor = snprintf(texto, capacidade, "%f", *parametro);
  }

  float height() { return 2.0f * rect.height + LABEL_MARGIN; }
};

struct Button
{
  std::string label;
  Rectangle rect;
  std::function<void()> action;

  Button(std::string label, Rectangle rect, std::function<void()> action)
      : label(label), rect(rect), action(action)
  {
  }

  void desenhar(Font font)
  {
    DrawRectangleRec(rect, BUTTON_COLOR);
    Vector2 text_offset =
        MeasureTextEx(font, label.c_str(), BUTTON_FONTSIZE, 3.0f);
    Vector2 rect_center = {rect.x + 0.5f * rect.width,
                           rect.y + 0.5f * rect.height};
    Vector2 text_pos =
        Vector2Subtract(rect_center, Vector2Scale(text_offset, 0.5));
    DrawTextEx(font, label.c_str(), text_pos, BUTTON_FONTSIZE, 3.0f, WHITE);
  }

  bool intersecao(const Vector2 &ponto)
  {
    return CheckCollisionPointRec(ponto, rect);
  }

  void move(const Vector2 &pos)
  {
    rect.x = pos.x;
    rect.y = pos.y;
  }

  float height() { return rect.height; }
};

struct Switch
{
  std::string label;
  Rectangle rect;
  bool *parametro = nullptr;

  Switch(std::string label, Rectangle rect, bool *parametro)
      : label(label), rect(rect), parametro(parametro)
  {
  }

  void desenhar(Font font)
  {
    float center_y = rect.y + 0.5f * rect.height;
    Color switch_color = *parametro ? SWITCH_ON_COLOR : SWITCH_OFF_COLOR;
    float small_r = 0.25f * rect.height, big_r = 0.5f * rect.height;
    float big_circle_x =
        *parametro ? rect.x + rect.width - big_r : rect.x + big_r;
    DrawCircle(rect.x + small_r, center_y, small_r, switch_color);
    DrawCircle(rect.x + rect.width - small_r, center_y, small_r, switch_color);
    DrawRectangleRec({rect.x + small_r,
                      center_y - small_r,
                      rect.width - 2.0f * small_r,
                      2.0f * small_r},
                     switch_color);
    DrawCircle(big_circle_x, center_y, big_r, switch_color);
    DrawTextEx(font,
               label.c_str(),
               {rect.x + rect.width + SWITCH_LABEL_MARGIN, rect.y},
               rect.height,
               3.0f,
               WHITE);
  }

  bool intersecao(const Vector2 &ponto)
  {
    return CheckCollisionPointRec(ponto, rect);
  }

  void atualizar_parametro() { *parametro = !*parametro; }

  void move(const Vector2 &pos)
  {
    rect.x = pos.x;
    rect.y = pos.y;
  }

  float height() { return rect.height; }
};

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
  omp_set_num_threads(8);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trabalho Final");
  SetTargetFPS(60);

  Font font = GetFontDefault();

  int objeto_selecionado = -1;

  tela = LoadRenderTexture(W_C, H_C);

  Button botaoAtualizar("Atualizar", (Rectangle){10, 10, 120, 40}, []()
                        {
    std::cout << "Atualizar cena.\n";
    renderizar(); });

  inicializar_objetosfinal(objetos, complexObjects);
  for (ObjetoComplexo &objeto_complexo : complexObjects)
  {
    flatten_objetos(objeto_complexo, objetos);
  }
  std::cout << "Objetos na cena: " << objetos.size() << "\n";

  inicializar_luzes();
  renderizar();

  // Loop principal simplificado
  // Loop principal simplificado com zoom via setas
  while (!WindowShouldClose())
  {
    // Verifica se a tecla de zoom in (seta para cima) foi pressionada
    if (IsKeyPressed(KEY_UP))
    {
      // Calcula um deslocamento (fator de zoom) – ajuste o valor 0.1f conforme necessário
      Vetor3d dv = (camera.lookAt - camera.position) * 0.1f;
      camera.position = camera.position + dv;
      camera.lookAt = camera.lookAt + dv;
      camera.updateCoordinates();
      renderizar(); // Atualiza a cena com os novos parâmetros da câmera
    }

    // Verifica se a tecla de zoom out (seta para baixo) foi pressionada
    if (IsKeyPressed(KEY_DOWN))
    {
      Vetor3d dv = (camera.lookAt - camera.position) * 0.1f;
      camera.position = camera.position - dv;
      camera.lookAt = camera.lookAt - dv;
      camera.updateCoordinates();
      renderizar(); // Atualiza a cena
    }

    // Verifica se o botão "Atualizar" foi clicado
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
      Vector2 mousePos = GetMousePosition();
      if (botaoAtualizar.intersecao(mousePos))
      {
        botaoAtualizar.action(); // Chama renderizar() via ação do botão
      }
    }

    BeginDrawing();
    {
      ClearBackground(BACKGROUND_COLOR);
      DrawTextureRec(tela.texture, (Rectangle){0.0f, 0.0f, W_C, -H_C}, (Vector2){0.0f, 0.0f}, WHITE);
      botaoAtualizar.desenhar(font);
    }
    EndDrawing();
  }

  CloseWindow();
  UnloadRenderTexture(tela);
  return 0;
}
