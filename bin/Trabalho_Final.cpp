#include <cmath>
#include <cstdio>
#include <functional>
#include <iostream>
#include <omp.h>
#include <raylib.h>
#include <raymath.h>
#include <string>
#include <vector>

// Ajuste os paths dos headers conforme necessário:
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

// ===================== Configurações da Janela e Render =====================
const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 500;
const int W_C = 500;  // Largura da área de render
const int H_C = 500;  // Altura da área de render

// ===================== Paleta e Layout do GUI =====================
const Color BACKGROUND_COLOR = { 44, 44, 44, 255 };
const Color PANEL_BG_COLOR   = { 30, 30, 50, 255 };
const Color BUTTON_COLOR     = { 150, 150, 180, 255 };
const Color TEXTBOX_COLOR    = { 80, 80, 100, 255 };
const Color TEXT_COLOR       = WHITE;
const float BUTTON_FONTSIZE  = 20.0f;
const float TEXTBOX_PADDING  = 2.0f;
const float LABEL_MARGIN     = 4.0f;
const float ELEMENT_MARGIN   = 6.0f;
const float TAB_LABEL_HEIGHT = 15.0f;
const float TAB_LABEL_PADDING = 2.0f;
const float TAB_LABEL_MARGIN_LEFT = 5.0f;
const float SCROLL_AMOUNT    = 10.0f;
const float TAB_SCROLL_AMOUNT = 5.0f;
const Color TAB_UNSELECTED_COLOR = { 84, 84, 84, 255 };

// ===================== Elementos do GUI =====================

struct TextBox {
    Rectangle rect;
    std::string label;
    char texto[30] = { 0 };
    size_t capacidade = sizeof(texto) / sizeof(texto[0]) - 1;
    size_t cursor = 0;
    float* parametro = nullptr;

    TextBox(std::string label, int x, int y, int width, int height, float* parametro)
      : label(label), parametro(parametro)
    {
      rect = { (float)x, (float)y, (float)width, (float)height };
    }
    TextBox(std::string label, Rectangle rect, float* parametro)
      : label(label), rect(rect), parametro(parametro)
    {
    }

    void desenhar(Font font)
    {
      // Desenha o rótulo acima da caixa
      DrawTextEx(font, label.c_str(), { rect.x, rect.y - 20 }, rect.height - 2 * TEXTBOX_PADDING, 3.0f, TEXT_COLOR);
      Rectangle box_rect = { rect.x, rect.y, rect.width, rect.height };
      DrawRectangleRec(box_rect, TEXTBOX_COLOR);
      DrawRectangleLinesEx(box_rect, 2, WHITE);
      Vector2 text_pos = { box_rect.x + TEXTBOX_PADDING, box_rect.y + TEXTBOX_PADDING };
      DrawTextEx(font, texto, text_pos, rect.height - 2 * TEXTBOX_PADDING, 3.0f, TEXT_COLOR);
    }

    bool intersecao(Vector2 ponto)
    {
      return CheckCollisionPointRec(ponto, rect);
    }

    void atualizar(int key, char key_char)
    {
      if (key == KEY_BACKSPACE) {
        if (cursor > 0)
          texto[--cursor] = '\0';
      } else if (key != 0 && cursor < capacidade && key != KEY_LEFT_SHIFT) {
        texto[cursor++] = key_char;
      }
    }

    void atualizar_parametro() { sscanf(texto, "%f", parametro); }

    void atualizar_texto() { cursor = snprintf(texto, capacidade, "%f", *parametro); }

    float height() { return rect.height + 20; } // inclui espaço para o rótulo
};

struct Button {
    std::string label;
    Rectangle rect;
    std::function<void()> action;

    Button(std::string label, Rectangle rect, std::function<void()> action)
      : label(label), rect(rect), action(action) {}

    void desenhar(Font font)
    {
      DrawRectangleRec(rect, BUTTON_COLOR);
      Vector2 text_size = MeasureTextEx(font, label.c_str(), BUTTON_FONTSIZE, 3.0f);
      Vector2 rect_center = { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f };
      Vector2 text_pos = Vector2Subtract(rect_center, Vector2Scale(text_size, 0.5f));
      DrawTextEx(font, label.c_str(), text_pos, BUTTON_FONTSIZE, 3.0f, TEXT_COLOR);
    }

    bool intersecao(Vector2 ponto) { return CheckCollisionPointRec(ponto, rect); }

    void move(const Vector2& pos) { rect.x = pos.x; rect.y = pos.y; }

    float height() { return rect.height; }
};

struct Switch {
    std::string label;
    Rectangle rect;
    bool* parametro = nullptr;

    Switch(std::string label, Rectangle rect, bool* parametro)
      : label(label), rect(rect), parametro(parametro) {}

    void desenhar(Font font)
    {
      float center_y = rect.y + rect.height * 0.5f;
      // Para evitar erro de compilação com listas braced, criamos variáveis temporárias:
      Color onColor = { 54, 200, 54, 255 };
      Color offColor = { 64, 64, 64, 255 };
      Color switch_color = *parametro ? onColor : offColor;
      float radius = rect.height * 0.5f;
      float circle_x = *parametro ? rect.x + rect.width - radius : rect.x + radius;
      DrawCircle(circle_x, center_y, radius, switch_color);
      DrawTextEx(font, label.c_str(), { rect.x + rect.width + 5, rect.y }, rect.height, 3.0f, TEXT_COLOR);
    }

    bool intersecao(const Vector2& ponto) { return CheckCollisionPointRec(ponto, rect); }

    void atualizar_parametro() { *parametro = !*parametro; }

    void move(const Vector2& pos) { rect.x = pos.x; rect.y = pos.y; }

    float height() { return rect.height; }
};

// ===================== Tab e TabbedPanel (mantendo controles para câmera e luz pontual) =====================
struct Tab {
    std::string label;
    std::vector<TextBox> textboxes;
    std::vector<Button> buttons;
    std::vector<Switch> switches;
    std::function<void()> renderizar;
    float left, top, lower_limit, bottom, scroll;
    int select_textbox = -1;

    Tab(float left, float top, float lower_limit, std::function<void()> renderizar)
      : left(left), top(top), lower_limit(lower_limit), bottom(top), scroll(lower_limit), renderizar(renderizar) {}

    void scroll_down()
    {
      scroll += SCROLL_AMOUNT;
      for (TextBox& textbox : textboxes)
        textbox.rect.y -= SCROLL_AMOUNT;
      for (Button& button : buttons)
        button.rect.y -= SCROLL_AMOUNT;
      for (Switch& sw : switches)
        sw.rect.y -= SCROLL_AMOUNT;
    }
    void scroll_up()
    {
      scroll -= SCROLL_AMOUNT;
      for (TextBox& textbox : textboxes)
        textbox.rect.y += SCROLL_AMOUNT;
      for (Button& button : buttons)
        button.rect.y += SCROLL_AMOUNT;
      for (Switch& sw : switches)
        sw.rect.y += SCROLL_AMOUNT;
    }

    void add_element(TextBox textbox)
    {
      textbox.rect.x = left;
      textbox.rect.y = bottom;
      bottom += textbox.height() + ELEMENT_MARGIN;
      textboxes.push_back(textbox);
    }
    void add_element(Button button)
    {
      button.rect.x = left;
      button.rect.y = bottom;
      bottom += button.height() + ELEMENT_MARGIN;
      buttons.push_back(button);
    }
    void add_element(Switch sw)
    {
      sw.rect.x = left;
      sw.rect.y = bottom;
      bottom += sw.height() + ELEMENT_MARGIN;
      switches.push_back(sw);
    }

    void desenhar(Font font)
    {
      for (auto& tb : textboxes)
        tb.desenhar(font);
      for (auto& bt : buttons)
        bt.desenhar(font);
      for (auto& sw : switches)
        sw.desenhar(font);
    }

    void intersecao(Vector2 mouse)
    {
      select_textbox = -1;
      for (int i = 0; i < textboxes.size(); ++i) {
        if (textboxes[i].intersecao(mouse)) {
          select_textbox = i;
          return;
        }
      }
      for (Button& button : buttons) {
        if (button.intersecao(mouse)) {
          button.action();
          renderizar();
          return;
        }
      }
      for (Switch& sw : switches) {
        if (sw.intersecao(mouse)) {
          sw.atualizar_parametro();
          renderizar();
          return;
        }
      }
    }

    void receber_input(int key, char key_char)
    {
      if (select_textbox >= 0)
        textboxes[select_textbox].atualizar(key, key_char);
    }

    void add_vector_controls(Vetor3d* ponto, std::string label)
    {
      Rectangle rect = { left, bottom, 260, 20 };
      TextBox x_box(TextFormat("%s.x", label.c_str()), rect, &ponto->x);
      TextBox y_box(TextFormat("%s.y", label.c_str()), rect, &ponto->y);
      TextBox z_box(TextFormat("%s.z", label.c_str()), rect, &ponto->z);
      add_element(x_box);
      add_element(y_box);
      add_element(z_box);
    }

    void add_color_controls(Vetor3d* color, std::string label)
    {
      Rectangle rect = { left, bottom, 260, 20 };
      TextBox r_box(TextFormat("%s.r", label.c_str()), rect, &color->x);
      TextBox g_box(TextFormat("%s.g", label.c_str()), rect, &color->y);
      TextBox b_box(TextFormat("%s.b", label.c_str()), rect, &color->z);
      add_element(r_box);
      add_element(g_box);
      add_element(b_box);
    }

    void add_material_controls(MaterialSimples* material, std::string label)
    {
      add_color_controls(&material->K_d, TextFormat("%s.K_d", label.c_str()));
      add_color_controls(&material->K_e, TextFormat("%s.K_e", label.c_str()));
      add_color_controls(&material->K_a, TextFormat("%s.K_a", label.c_str()));
      Rectangle rect = { left, bottom, 260, 20 };
      TextBox m_box(TextFormat("%s.m", label.c_str()), rect, &material->m);
      add_element(m_box);
    }

    void add_object_controls(Objeto* objeto, std::string label)
    {
      // Adiciona controle de visibilidade e material
      Switch vis_switch("Visível", { left, bottom, 30, 20 }, &objeto->visivel);
      add_element(vis_switch);
      add_material_controls(&objeto->material, label);
      for (TextBox& textbox : textboxes)
        textbox.atualizar_texto();
    }

    void add_camera_controls(Camera3de* camera, std::string label)
    {
      add_vector_controls(&camera->position, TextFormat("%s.position", label.c_str()));
      add_vector_controls(&camera->lookAt, TextFormat("%s.lookAt", label.c_str()));
      add_vector_controls(&camera->Up, TextFormat("%s.Up", label.c_str()));
      Rectangle d_box = { left, bottom, 260, 20 };
      TextBox d_text(TextFormat("%s.d", label.c_str()), d_box, &camera->d);
      add_element(d_text);
      Rectangle xmin_box = { left, bottom, 260, 20 };
      TextBox xmin_text(TextFormat("%s.x_min", label.c_str()), xmin_box, &camera->xmin);
      add_element(xmin_text);
      Rectangle ymin_box = { left, bottom, 260, 20 };
      TextBox ymin_text(TextFormat("%s.y_min", label.c_str()), ymin_box, &camera->ymin);
      add_element(ymin_text);
      Rectangle xmax_box = { left, bottom, 260, 20 };
      TextBox xmax_text(TextFormat("%s.x_max", label.c_str()), xmax_box, &camera->xmax);
      add_element(xmax_text);
      Rectangle ymax_box = { left, bottom, 260, 20 };
      TextBox ymax_text(TextFormat("%s.y_max", label.c_str()), ymax_box, &camera->ymax);
      add_element(ymax_text);
      for (TextBox& textbox : textboxes)
        textbox.atualizar_texto();
    }

    void add_light_controls(iluminacao::FontePontual* fonte, std::string label)
    {
      add_vector_controls(&fonte->posicao, TextFormat("%s.posicao", label.c_str()));
      add_vector_controls(&fonte->intensidade, TextFormat("%s.intensidade", label.c_str()));
      for (TextBox& textbox : textboxes)
        textbox.atualizar_texto();
      Rectangle btn_rect = { left, bottom, 260, 30 };
      Button atualizar_btn("Atualizar", btn_rect, [this] {
        for (TextBox& textbox : this->textboxes)
          textbox.atualizar_parametro();
      });
      add_element(atualizar_btn);
      Rectangle switch_rect = { left, bottom, 30, 20 };
      Switch acesa_switch("Acesa", switch_rect, &fonte->acesa);
      add_element(acesa_switch);
    }
};

struct TabbedPanel {
    std::vector<Tab> tabs;
    std::vector<std::string> labels;
    int selected_tab = -1;
    Rectangle rect;
    float scroll;
    float right;
    Font font;
    std::function<void()> renderizar;

    TabbedPanel(Rectangle rect, Font font, std::function<void()> renderizar)
      : rect(rect), scroll(0), right(0), font(font), renderizar(renderizar) {}

    void scroll_tabs_right()
    {
      scroll += TAB_SCROLL_AMOUNT;
    }
    void scroll_tabs_left()
    {
      scroll -= TAB_SCROLL_AMOUNT;
      if (scroll < 0) scroll = 0;
    }
    void scroll_up()
    {
      if (selected_tab >= 0)
        tabs[selected_tab].scroll_up();
    }
    void scroll_down()
    {
      if (selected_tab >= 0)
        tabs[selected_tab].scroll_down();
    }
    void desenhar()
    {
      if (selected_tab >= 0)
        tabs[selected_tab].desenhar(font);
      // Desenha os rótulos das abas
      DrawRectangleRec({ rect.x, rect.y, rect.width, TAB_LABEL_HEIGHT }, BACKGROUND_COLOR);
      float left_pos = rect.x - scroll;
      float tab_fontsize = TAB_LABEL_HEIGHT - 2 * TAB_LABEL_PADDING;
      for (int i = 0; i < labels.size(); ++i) {
          std::string& label = labels[i];
          Vector2 text_size = MeasureTextEx(font, label.c_str(), tab_fontsize, 3.0f);
          Rectangle tab_rect = { left_pos, rect.y, text_size.x + 2 * TAB_LABEL_PADDING, TAB_LABEL_HEIGHT };
          Color bg = (selected_tab == i) ? BUTTON_COLOR : TAB_UNSELECTED_COLOR;
          DrawRectangleRec(tab_rect, bg);
          DrawTextEx(font, label.c_str(), { left_pos + TAB_LABEL_PADDING, rect.y + TAB_LABEL_PADDING }, tab_fontsize, 3.0f, TEXT_COLOR);
          left_pos += text_size.x + 2 * TAB_LABEL_PADDING + TAB_LABEL_MARGIN_LEFT;
      }
    }
    void intersecao(Vector2 mouse)
    {
      float left_pos = rect.x - scroll;
      for (int i = 0; i < labels.size(); ++i) {
          float tab_fontsize = TAB_LABEL_HEIGHT - 2 * TAB_LABEL_PADDING;
          Vector2 text_size = MeasureTextEx(font, labels[i].c_str(), tab_fontsize, 3.0f);
          Rectangle tab_rect = { left_pos, rect.y, text_size.x + 2 * TAB_LABEL_PADDING, TAB_LABEL_HEIGHT };
          if (CheckCollisionPointRec(mouse, tab_rect)) {
              selected_tab = i;
              return;
          }
          left_pos += text_size.x + 2 * TAB_LABEL_PADDING + TAB_LABEL_MARGIN_LEFT;
      }
      if (selected_tab >= 0)
          tabs[selected_tab].intersecao(mouse);
    }
    void receber_input(int key, char key_char)
    {
      if (selected_tab >= 0)
          tabs[selected_tab].receber_input(key, key_char);
    }
    void add_tab(std::string label)
    {
      selected_tab = tabs.size();
      tabs.emplace_back(rect.x + 20, rect.y + 10 + TAB_LABEL_HEIGHT, rect.y + rect.height, renderizar);
      labels.push_back(label);
      float tab_fontsize = TAB_LABEL_HEIGHT - 2 * TAB_LABEL_PADDING;
      Vector2 text_size = MeasureTextEx(font, label.c_str(), tab_fontsize, 3.0f);
      right += text_size.x + 2 * TAB_LABEL_PADDING + TAB_LABEL_MARGIN_LEFT;
    }
    void add_element_tab(int tab_idx, TextBox& textbox) { tabs[tab_idx].add_element(textbox); }
    void add_element_tab(int tab_idx, Button& button) { tabs[tab_idx].add_element(button); }
    void add_element_tab(int tab_idx, Switch& sw) { tabs[tab_idx].add_element(sw); }
    void add_tab_luz(iluminacao::FontePontual* fonte, std::string label)
    {
      add_tab(label);
      tabs[selected_tab].add_light_controls(fonte, label);
    }
    void add_tab_objeto(Objeto* objeto, std::string label)
    {
      add_tab(label);
      tabs[selected_tab].add_object_controls(objeto, label);
    }
    void add_tab_camera(Camera3de* camera, std::string label)
    {
      add_tab(label);
      tabs[selected_tab].add_camera_controls(camera, label);
    }
};

// ===================== Outras Configurações de Render =====================
float W_J = 60.0f, H_J = 60.0f;
const int nLin = 500, nCol = 500;
float d = 30.0f;
double deltinhax = W_J / nCol, deltinhay = H_J / nLin;
Vetor3d Ponto_Superior_Esquerdo = { -W_J * 0.5f, W_J * 0.5f, -d };

std::vector<Color> pixel_buffer(nLin * nCol, WHITE);

// ===================== Iluminação e Câmera =====================
std::vector<iluminacao::FontePontual> fontes_pontuais;
std::vector<std::string> fontes_pontuais_labels;
Vetor3d I_A = { 0.4f, 0.4f, 0.4f };

std::vector<ObjetoComplexo> complexObjects;
std::vector<Objeto> objetos;

RenderTexture2D tela;

// Configuração inicial da câmera
Vetor3d Eye = { 500.0f, 125.0f, 1700.0f };
Vetor3d At = { 650.0f, 10.0f, 700.0f };
Vetor3d Up = { 500.0f, 200.0f, 1700.0f };
Camera3de camera(Eye, At, Up);

// ===================== Função de Renderização =====================
void renderizar()
{
    // Limpa o buffer de pixels
    for (auto &px : pixel_buffer)
        px = WHITE;

    Matriz M_cw = camera.getMatrixCameraWorld();
    Vetor3d PSE = (M_cw * camera.get_PSE().ponto4d()).vetor3d();
    Vetor3d right = (M_cw * Vetor3d{1,0,0}.vetor4d()).vetor3d();
    Vetor3d down  = (M_cw * Vetor3d{0,-1,0}.vetor4d()).vetor3d();

    deltinhax = camera.get_W_J() / nCol;
    deltinhay = camera.get_H_J() / nLin;

    BeginTextureMode(tela);
    ClearBackground(BLACK);

    for (int i = 0; i < nLin; ++i) {
        for (int j = 0; j < nCol; ++j) {
            Vetor3d P = PSE + right * (deltinhax * (j + 0.5f))
                            + down * (deltinhay * (i + 0.5f));
            Vetor3d dr = (P - camera.position).normalizado();
            Raio raio(camera.position, dr);
            auto [t, idx] = calcular_intersecao(raio, objetos);
            if (t > 0.0f) {
                Vetor3d Pt = raio.no_ponto(t);
                Vetor3d normal = objetos[idx].normal(Pt);
                MaterialSimples mat;
                std::visit([&](auto &objTipo){
                    using T = std::decay_t<decltype(objTipo)>;
                    if constexpr(std::is_same_v<T, PlanoTextura>)
                        mat = objTipo.material(Pt);
                    else
                        mat = objetos[idx].material;
                }, objetos[idx].obj);
                // Apenas luz pontual (removidas demais)
                Vetor3d I_total = iluminacao::luz_ambiente(I_A, mat.K_a);
                for (const iluminacao::FontePontual& fonte : fontes_pontuais) {
                    if (!fonte.acesa) continue;
                    Vetor3d dr_luz = fonte.posicao - Pt;
                    float dist_luz = dr_luz.tamanho();
                    if (dist_luz < 1e-6f) continue;
                    // Substituindo operador '/' por multiplicação
                    dr_luz = dr_luz * (1.0f / dist_luz);
                    Raio raio_luz(Pt, dr_luz);
                    auto [t_luz, _] = calcular_intersecao(raio_luz, objetos, idx);
                    if (t_luz < 0.0f || t_luz > dist_luz)
                        I_total = I_total + modelo_phong(Pt, raio.dr, normal, fonte, mat);
                }
                pixel_buffer[i * nCol + j] = {
                    static_cast<unsigned char>(fmin(I_total.x * 255, 255)),
                    static_cast<unsigned char>(fmin(I_total.y * 255, 255)),
                    static_cast<unsigned char>(fmin(I_total.z * 255, 255)),
                    255
                };
            }
        }
    }
    for (int i = 0; i < nLin; ++i)
        for (int j = 0; j < nCol; ++j)
            DrawPixel(j, i, pixel_buffer[i * nCol + j]);
    EndTextureMode();
}

// ===================== Inicializa Luz Pontual =====================
void inicializar_luzes()
{
    fontes_pontuais.push_back(iluminacao::FontePontual({600, 200, 1500}, {0.8f, 0.8f, 0.8f}));
    fontes_pontuais_labels.push_back("Luz Pontual");
}

// ===================== Função Principal =====================
int main()
{
    omp_set_num_threads(8);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Trabalho Final");
    SetTargetFPS(60);

    Font font = GetFontDefault();
    tela = LoadRenderTexture(W_C, H_C);

    inicializar_objetosfinal(objetos, complexObjects);
    for (ObjetoComplexo& obj_complexo : complexObjects)
        flatten_objetos(obj_complexo, objetos);
    std::cout << "Objetos na cena: " << objetos.size() << "\n";

    inicializar_luzes();
    camera.updateCoordinates();
    renderizar();

    // Cria o painel de abas – somente para câmera e luz pontual
    TabbedPanel panel({500, 0, 300, 450}, font, [](){ renderizar(); });
    panel.add_tab_camera(&camera, "Câmera");
    panel.add_tab_luz(&fontes_pontuais[0], fontes_pontuais_labels[0]);

    int objeto_selecionado = -1;

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (mouse.x < W_C && mouse.y < H_C) {
                int x_pos = mouse.x, y_pos = mouse.y;
                // Calcula o raio com base na posição de render
                Vetor3d P = (camera.getMatrixCameraWorld() * camera.get_PSE().ponto4d()).vetor3d();
                Vetor3d dr = (P - camera.position).normalizado();
                Raio raio(camera.position, dr);
                auto [t, idx] = calcular_intersecao(raio, objetos);
                if (t > 0.0f) {
                    objeto_selecionado = idx;
                    TraceLog(LOG_INFO, "Objeto selecionado = %d", objeto_selecionado);
                    panel.add_tab_objeto(&objetos[objeto_selecionado], TextFormat("obj%d", objeto_selecionado));
                }
            } else {
                panel.intersecao(mouse);
            }
        }
        if (IsKeyDown(KEY_UP))
            panel.scroll_up();
        else if (IsKeyDown(KEY_DOWN))
            panel.scroll_down();
        else if (IsKeyDown(KEY_RIGHT))
            panel.scroll_tabs_right();
        else if (IsKeyDown(KEY_LEFT))
            panel.scroll_tabs_left();

        int key = GetKeyPressed();
        char key_char = GetCharPressed();
        panel.receber_input(key, key_char);

        BeginDrawing();
            ClearBackground(BACKGROUND_COLOR);
            panel.desenhar();
            DrawTextureRec(tela.texture, {0, 0, (float)W_C, -H_C}, {0,0}, WHITE);
        EndDrawing();
    }
    CloseWindow();
    UnloadRenderTexture(tela);
    return 0;
}
