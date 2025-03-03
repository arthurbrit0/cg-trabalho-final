#include <cstdio>
#include <math.h>

#include <raylib.h>
#include <raymath.h>

#include <functional>
#include <string>
#include <vector>

#include "./src/Camera/Camera3de.h"
#include "./src/Cilindro/Cilindro.h"
#include "./src/Circulo/Circulo.h"
#include "./src/Cone/Cone.h"
#include "./src/Esfera/Esfera.h"
#include "./src/Iluminacao/Iluminacao.h"
#include "./src/Material/Material.h"
#include "./src/Objeto/Objeto.h"
#include "./src/Plano/Plano.h"
#include "./src/Raio/Raio.h"
#include "./src/Triangulo/Triangulo.h"
#include "funcoes_auxiliares.h"
#include "src/calcular_intersecao.h"
using namespace funcoes_auxiliares;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 500;
const Color BACKGROUND_COLOR = { 44, 44, 44, 255 };
const Color TEXTBOX_COLOR = { 64, 64, 64, 255 };
const Color BUTTON_COLOR = { 54, 54, 54, 255 };
const Color SWITCH_ON_COLOR = { 54, 200, 54, 255 };
const Color SWITCH_OFF_COLOR = { 64, 64, 64, 255 };
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
const Color TAB_UNSELECTED_COLOR = { 84, 84, 84, 255 };

struct TextBox
{
  Rectangle rect;
  std::string label;
  char texto[30] = { 0 };
  size_t capacidade = sizeof(texto) / sizeof(texto[0]) - 1;
  size_t cursor = 0;
  float* parametro = nullptr;

  TextBox(std::string label,
          int x,
          int y,
          int width,
          int height,
          float* parametro)
    : label(label)
    , parametro(parametro)
  {
    rect = (Rectangle){ (float)x, (float)y, (float)width, (float)height };
  }

  TextBox(std::string label, Rectangle rect, float* parametro)
    : label(label)
    , rect(rect)
    , parametro(parametro)
  {
  }

  void desenhar(Font font)
  {
    float font_size = rect.height - 2 * TEXTBOX_PADDING;
    float label_height = rect.height;
    Vector2 label_pos = { rect.x, rect.y };
    DrawTextEx(font, label.c_str(), label_pos, font_size, 3.0f, WHITE);
    Rectangle box_rect = {
      rect.x, rect.y + label_height + LABEL_MARGIN, rect.width, rect.height
    };
    DrawRectangleRec(box_rect, TEXTBOX_COLOR);
    Vector2 text_pos = { box_rect.x + TEXTBOX_PADDING,
                         box_rect.y + TEXTBOX_PADDING };
    DrawTextEx(font, texto, text_pos, font_size, 3.0f, WHITE);
  }

  bool intersecao(Vector2 ponto)
  {
    float label_height = rect.height;
    Rectangle box_rect = {
      rect.x, rect.y + label_height + LABEL_MARGIN, rect.width, rect.height
    };
    return CheckCollisionPointRec(ponto, box_rect);
  }

  void move(const Vector2& pos)
  {
    rect.x = pos.x;
    rect.y = pos.y;
  }

  void atualizar(int key, char key_char)
  {
    if (key == KEY_BACKSPACE) {
      if (cursor > 0) {
        texto[--cursor] = '\0';
      }
    } else if (key != 0 && cursor < capacidade && key != KEY_LEFT_SHIFT) {
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
    : label(label)
    , rect(rect)
    , action(action)
  {
  }

  void desenhar(Font font)
  {
    DrawRectangleRec(rect, BUTTON_COLOR);
    Vector2 text_offset =
      MeasureTextEx(font, label.c_str(), BUTTON_FONTSIZE, 3.0f);
    Vector2 rect_center = { rect.x + 0.5f * rect.width,
                            rect.y + 0.5f * rect.height };
    Vector2 text_pos =
      Vector2Subtract(rect_center, Vector2Scale(text_offset, 0.5));
    DrawTextEx(font, label.c_str(), text_pos, BUTTON_FONTSIZE, 3.0f, WHITE);
  }

  bool intersecao(const Vector2& ponto)
  {
    return CheckCollisionPointRec(ponto, rect);
  }

  void move(const Vector2& pos)
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
  bool* parametro = nullptr;

  Switch(std::string label, Rectangle rect, bool* parametro)
    : label(label)
    , rect(rect)
    , parametro(parametro)
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
    DrawRectangleRec({ rect.x + small_r,
                       center_y - small_r,
                       rect.width - 2.0f * small_r,
                       2.0f * small_r },
                     switch_color);
    DrawCircle(big_circle_x, center_y, big_r, switch_color);
    DrawTextEx(font,
               label.c_str(),
               { rect.x + rect.width + SWITCH_LABEL_MARGIN, rect.y },
               rect.height,
               3.0f,
               WHITE);
  }

  bool intersecao(const Vector2& ponto)
  {
    return CheckCollisionPointRec(ponto, rect);
  }

  void atualizar_parametro() { *parametro = !*parametro; }

  void move(const Vector2& pos)
  {
    rect.x = pos.x;
    rect.y = pos.y;
  }

  float height() { return rect.height; }
};

struct Tab
{
  std::string label;
  std::vector<TextBox> textboxes;
  std::vector<Button> buttons;
  std::vector<Switch> switches;
  std::function<void()> renderizar;

  float left;
  float top;
  float lower_limit;
  float bottom;
  float scroll;

  int select_textbox = -1;

  Tab(float left,
      float top,
      float lower_limit,
      std::function<void()> renderizar)
    : left(left)
    , bottom(top)
    , top(top)
    , lower_limit(lower_limit)
    , scroll(lower_limit)
    , renderizar(renderizar)
  {
  }

  void scroll_down()
  {
    scroll += SCROLL_AMOUNT;
    if (scroll > bottom) {
      scroll -= SCROLL_AMOUNT;
      return;
    }

    for (TextBox& textbox : textboxes) {
      textbox.move({ textbox.rect.x, textbox.rect.y - SCROLL_AMOUNT });
    }

    for (Button& button : buttons) {
      button.move({ button.rect.x, button.rect.y - SCROLL_AMOUNT });
    }

    for (Switch& switch_ : switches) {
      switch_.move({ switch_.rect.x, switch_.rect.y - SCROLL_AMOUNT });
    }
  }

  void scroll_up()
  {
    scroll -= SCROLL_AMOUNT;
    if (scroll < lower_limit) {
      scroll += SCROLL_AMOUNT;
      return;
    }

    for (TextBox& textbox : textboxes) {
      textbox.move({ textbox.rect.x, textbox.rect.y + SCROLL_AMOUNT });
    }

    for (Button& button : buttons) {
      button.move({ button.rect.x, button.rect.y + SCROLL_AMOUNT });
    }

    for (Switch& switch_ : switches) {
      switch_.move({ switch_.rect.x, switch_.rect.y + SCROLL_AMOUNT });
    }
  }

  void add_element(TextBox& textbox)
  {
    textbox.move({ left, bottom });
    bottom += textbox.height() + ELEMENT_MARGIN;
    textboxes.push_back(textbox);
  }

  void add_element(Button& button)
  {
    button.move({ left, bottom });
    bottom += button.height() + ELEMENT_MARGIN;
    buttons.push_back(button);
  }

  void add_element(Switch& switch_)
  {
    switch_.move({ left, bottom });
    bottom += switch_.height() + ELEMENT_MARGIN;
    switches.push_back(switch_);
  }

  void desenhar(Font font)
  {
    for (auto& element : textboxes) {
      element.desenhar(font);
    }
    for (auto& element : buttons) {
      element.desenhar(font);
    }
    for (auto& element : switches) {
      element.desenhar(font);
    }
  }

  void intersecao(Vector2 mouse)
  {
    select_textbox = -1;
    for (int i = 0; i < textboxes.size(); ++i) {
      TextBox& textbox = textboxes[i];
      if (textbox.intersecao(mouse)) {
        select_textbox = i;
        return;
      }
    }
    for (int i = 0; i < buttons.size(); ++i) {
      Button& button = buttons[i];
      if (button.intersecao(mouse)) {
        button.action();
        renderizar();
        return;
      }
    }
    for (int i = 0; i < switches.size(); ++i) {
      Switch& switch_ = switches[i];
      if (switch_.intersecao(mouse)) {
        switch_.atualizar_parametro();
        renderizar();
        return;
      }
    }
  }

  void receber_input(int key, char key_char)
  {
    if (select_textbox >= 0) {
      textboxes[select_textbox].atualizar(key, key_char);
    }
  }

  void add_vector_controls(Vetor3d* ponto, std::string label)
  {
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    TextBox x_box(TextFormat("%s.x", label.c_str()), rect, &ponto->x);
    TextBox y_box(TextFormat("%s.y", label.c_str()), rect, &ponto->y);
    TextBox z_box(TextFormat("%s.z", label.c_str()), rect, &ponto->z);
    add_element(x_box);
    add_element(y_box);
    add_element(z_box);
  }

  void add_color_controls(Vetor3d* color, std::string label)
  {
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    TextBox r_box(TextFormat("%s.r", label.c_str()), rect, &color->x);
    TextBox g_box(TextFormat("%s.g", label.c_str()), rect, &color->y);
    TextBox b_box(TextFormat("%s.b", label.c_str()), rect, &color->z);
    add_element(r_box);
    add_element(g_box);
    add_element(b_box);
  }

  void add_material_controls(MaterialSimples* material, std::string label)
  {
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    add_color_controls(&material->K_d, TextFormat("%s.K_d", label.c_str()));
    add_color_controls(&material->K_e, TextFormat("%s.K_e", label.c_str()));
    add_color_controls(&material->K_a, TextFormat("%s.K_a", label.c_str()));
    TextBox m_box(TextFormat("%s.m", label.c_str()), rect, &material->m);
    add_element(m_box);
  }

  void add_object_controls(Objeto* objeto, std::string label)
  {
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    Rectangle btn_rect = { 0.0f, 0.0f, 260.0f, 30.0f };
    Rectangle switch_rect = { 0.0f, 0.0f, 30.0f, 20.0f };
    std::visit(
      [this, objeto, label, btn_rect, switch_rect](auto&& obj) {
        using T = std::decay_t<decltype(obj)>;
        Switch visivel_switch("Visivel", switch_rect, &objeto->visivel);
        add_element(visivel_switch);
        add_object_controls(&obj, label);
        if constexpr (!std::is_same_v<T, PlanoTextura>) {
          add_material_controls(&objeto->material, label);
        }
        if constexpr (!std::is_same_v<T, Malha>) {
          Button atualizar_btn("Atualizar", btn_rect, [this] {
            for (TextBox& textbox : this->textboxes)
              textbox.atualizar_parametro();
          });
          add_element(atualizar_btn);
        } else {
          Button atualizar_btn("Atualizar", btn_rect, [this, &obj] {
            Vetor3d ancora_antiga = obj.ancora;
            for (TextBox& textbox : this->textboxes)
              textbox.atualizar_parametro();
            Vetor3d ancora_nova = obj.ancora;
            TraceLog(LOG_INFO,
                     "ancora_antiga = (%f, %f, %f)",
                     ancora_antiga.x,
                     ancora_antiga.y,
                     ancora_antiga.z);
            TraceLog(LOG_INFO,
                     "ancora_nova = (%f, %f, %f)",
                     ancora_nova.x,
                     ancora_nova.y,
                     ancora_nova.z);
            obj.transformar(Matriz::translacao(ancora_nova - ancora_antiga));
            obj.ancora = ancora_nova;
          });
          add_element(atualizar_btn);
        }
        for (TextBox& textbox : textboxes)
          textbox.atualizar_texto();
      },
      objeto->obj);
  }

  void add_object_controls(Plano* plano, std::string label)
  {
    add_vector_controls(&plano->normal, TextFormat("%s.normal", label.c_str()));
    add_vector_controls(&plano->ponto, TextFormat("%s.ponto", label.c_str()));
  }

  void add_object_controls(Esfera* esfera, std::string label)
  {
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    Rectangle btn_rect = { 0.0f, 0.0f, 260.0f, 30.0f };
    add_vector_controls(&esfera->centro,
                        TextFormat("%s.centro", label.c_str()));
    TextBox radius_box(
      TextFormat("%s.raio", label.c_str()), rect, &esfera->raio);
    add_element(radius_box);
  }

  void add_object_controls(Cilindro* cilindro, std::string label)
  {
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    Rectangle btn_rect = { 0.0f, 0.0f, 260.0f, 30.0f };
    add_vector_controls(&cilindro->centro,
                        TextFormat("%s.centro", label.c_str()));
    TextBox radius_box(
      TextFormat("%s.raio", label.c_str()), rect, &cilindro->raio);
    add_element(radius_box);
    TextBox height_box(
      TextFormat("%s.altura", label.c_str()), rect, &cilindro->altura);
    add_element(height_box);
    add_vector_controls(&cilindro->direcao,
                        TextFormat("%s.direcao", label.c_str()));
  }

  void add_object_controls(Cone* cone, std::string label)
  {
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    Rectangle btn_rect = { 0.0f, 0.0f, 260.0f, 30.0f };
    add_vector_controls(&cone->centro, TextFormat("%s.centro", label.c_str()));
    TextBox radius_box(TextFormat("%s.raio", label.c_str()), rect, &cone->raio);
    add_element(radius_box);
    TextBox height_box(
      TextFormat("%s.altura", label.c_str()), rect, &cone->altura);
    add_element(height_box);
    add_vector_controls(&cone->direcao,
                        TextFormat("%s.direcao", label.c_str()));
  }

  void add_object_controls(Circulo* circulo, std::string label)
  {
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    add_vector_controls(&circulo->centro,
                        TextFormat("%s.centro", label.c_str()));
    TextBox radius_box(
      TextFormat("%s.raio", label.c_str()), rect, &circulo->raio);
    add_element(radius_box);
    add_vector_controls(&circulo->normal,
                        TextFormat("%s.normal", label.c_str()));
  }

  void add_object_controls(Triangulo* triangulo, std::string label) {}

  void add_object_controls(PlanoTextura* plano_textura, std::string label)
  {
    add_vector_controls(&plano_textura->normal,
                        TextFormat("%s.normal", label.c_str()));
    add_vector_controls(&plano_textura->ponto,
                        TextFormat("%s.ponto", label.c_str()));
  }

  void add_object_controls(Malha* malha, std::string label)
  {
    add_vector_controls(&malha->ancora, TextFormat("%s.ancora", label.c_str()));
  }

  void add_camera_controls(Camera3de* camera, std::string label)
  {
    add_vector_controls(&camera->position,
                        TextFormat("%s.position", label.c_str()));
    add_vector_controls(&camera->lookAt,
                        TextFormat("%s.lookAt", label.c_str()));
    add_vector_controls(&camera->Up, TextFormat("%s.Up", label.c_str()));
    TextBox d_box(TextFormat("%s.d", label.c_str()),
                  { 0.0f, 0.0f, 260.0f, 20.0f },
                  &camera->d);
    TextBox xmin_box(TextFormat("%s.x_min", label.c_str()),
                     { 0.0f, 0.0f, 260.0f, 20.0f },
                     &camera->xmin);
    TextBox ymin_box(TextFormat("%s.y_min", label.c_str()),
                     { 0.0f, 0.0f, 260.0f, 20.0f },
                     &camera->ymin);
    TextBox xmax_box(TextFormat("%s.x_max", label.c_str()),
                     { 0.0f, 0.0f, 260.0f, 20.0f },
                     &camera->xmax);
    TextBox ymax_box(TextFormat("%s.y_max", label.c_str()),
                     { 0.0f, 0.0f, 260.0f, 20.0f },
                     &camera->ymax);
    add_element(d_box);
    add_element(xmin_box);
    add_element(ymin_box);
    add_element(xmax_box);
    add_element(ymax_box);
    for (TextBox& textbox : textboxes)
      textbox.atualizar_texto();
  }

  void add_light_controls(iluminacao::FontePontual* fonte, std::string label)
  {
    Rectangle switch_rect = { 0.0f, 0.0f, 30.0f, 20.0f };
    Rectangle btn_rect = { 0.0f, 0.0f, 260.0f, 30.0f };
    add_vector_controls(&fonte->posicao,
                        TextFormat("%s.posicao", label.c_str()));
    add_vector_controls(&fonte->intensidade,
                        TextFormat("%s.intensidade", label.c_str()));
    for (TextBox& textbox : textboxes)
      textbox.atualizar_texto();
    Button atualizar_btn("Atualizar", btn_rect, [this] {
      for (TextBox& textbox : this->textboxes)
        textbox.atualizar_parametro();
    });
    add_element(atualizar_btn);
    Switch acesa_switch("Acesa", switch_rect, &fonte->acesa);
    add_element(acesa_switch);
  }
  void add_light_controls(iluminacao::FonteDirecional* fonte, std::string label)
  {
    Rectangle switch_rect = { 0.0f, 0.0f, 30.0f, 20.0f };
    Rectangle btn_rect = { 0.0f, 0.0f, 260.0f, 30.0f };
    add_vector_controls(&fonte->direcao,
                        TextFormat("%s.direcao", label.c_str()));
    add_vector_controls(&fonte->intensidade,
                        TextFormat("%s.intensidade", label.c_str()));
    for (TextBox& textbox : textboxes)
      textbox.atualizar_texto();
    Button atualizar_btn("Atualizar", btn_rect, [this] {
      for (TextBox& textbox : this->textboxes)
        textbox.atualizar_parametro();
    });
    add_element(atualizar_btn);
    Switch acesa_switch("Acesa", switch_rect, &fonte->acesa);
    add_element(acesa_switch);
  }
  void add_light_controls(iluminacao::FonteSpot* fonte, std::string label)
  {
    Rectangle switch_rect = { 0.0f, 0.0f, 30.0f, 20.0f };
    Rectangle btn_rect = { 0.0f, 0.0f, 260.0f, 30.0f };
    Rectangle rect = { 0.0f, 0.0f, 260.0f, 20.0f };
    add_vector_controls(&fonte->posicao,
                        TextFormat("%s.posicao", label.c_str()));
    add_vector_controls(&fonte->intensidade,
                        TextFormat("%s.intensidade", label.c_str()));
    add_vector_controls(&fonte->direcao,
                        TextFormat("%s.direcao", label.c_str()));
    TextBox angle_box(
      TextFormat("%s.cos_beta", label.c_str()), rect, &fonte->cos_beta);
    add_element(angle_box);
    for (TextBox& textbox : textboxes)
      textbox.atualizar_texto();
    Button atualizar_btn("Atualizar", btn_rect, [this] {
      for (TextBox& textbox : this->textboxes)
        textbox.atualizar_parametro();
    });
    add_element(atualizar_btn);
    Switch acesa_switch("Acesa", switch_rect, &fonte->acesa);
    add_element(acesa_switch);
  }
};

struct TabbedPanel
{
  std::vector<Tab> tabs;
  std::vector<std::string> labels;
  int selected_tab = -1;
  Rectangle rect;
  float scroll;
  float right_limit;
  float right = 0.0f;
  Font font;
  std::function<void()> renderizar;

  TabbedPanel(Rectangle rect, Font font, std::function<void()> renderizar)
    : rect(rect)
    , scroll(rect.width)
    , right_limit(rect.width)
    , font(font)
    , renderizar(renderizar)
  {
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

  void scroll_tabs_right()
  {
    scroll += TAB_SCROLL_AMOUNT;
    if (scroll > right) {
      scroll -= TAB_SCROLL_AMOUNT;
    }
  }

  void scroll_tabs_left()
  {
    scroll -= TAB_SCROLL_AMOUNT;
    if (scroll < right_limit) {
      scroll += TAB_SCROLL_AMOUNT;
    }
  }

  void desenhar()
  {
    if (selected_tab >= 0)
      tabs[selected_tab].desenhar(font);
    DrawRectangleRec({ rect.x, rect.y, rect.width, TAB_LABEL_HEIGHT },
                     BACKGROUND_COLOR);
    float left = rect.x - scroll + rect.width;
    float tab_fontsize = TAB_LABEL_HEIGHT - 2.0f * TAB_LABEL_PADDING;
    for (int i = 0; i < labels.size(); ++i) {
      std::string& label = labels[i];
      Vector2 text_size =
        MeasureTextEx(font, label.c_str(), tab_fontsize, 3.0f);
      if (selected_tab != i) {
        DrawRectangleRec({ left,
                           rect.y,
                           text_size.x + 2.0f * TAB_LABEL_PADDING,
                           TAB_LABEL_HEIGHT },
                         TAB_UNSELECTED_COLOR);
      }
      DrawTextEx(font,
                 label.c_str(),
                 { left + TAB_LABEL_PADDING, rect.y + TAB_LABEL_PADDING },
                 tab_fontsize,
                 3.0f,
                 WHITE);
      left += text_size.x + 2.0f * TAB_LABEL_PADDING + TAB_LABEL_MARGIN_LEFT;
    }
  }

  void intersecao(Vector2 mouse)
  {
    float left = rect.x - scroll + rect.width;
    for (int i = 0; i < labels.size(); ++i) {
      std::string& label = labels[i];
      float tab_fontsize = TAB_LABEL_HEIGHT - 2.0f * TAB_LABEL_PADDING;
      Vector2 text_size =
        MeasureTextEx(font, label.c_str(), tab_fontsize, 3.0f);
      Rectangle tab_rect = {
        left, rect.y, text_size.x + 2.0f * TAB_LABEL_PADDING, TAB_LABEL_HEIGHT
      };
      if (CheckCollisionPointRec(mouse, tab_rect)) {
        selected_tab = i;
        TraceLog(LOG_INFO, "select_tab = %d", selected_tab);
        return;
      }
      left += text_size.x + 2.0f * TAB_LABEL_PADDING + TAB_LABEL_MARGIN_LEFT;
    }
    if (selected_tab >= 0) {
      tabs[selected_tab].intersecao(mouse);
    }
  }

  void add_tab(std::string label)
  {
    selected_tab = tabs.size();
    tabs.emplace_back(
      rect.x + 20.0f, rect.y + 10.0f + TAB_LABEL_HEIGHT, 500.0f, renderizar);
    labels.emplace_back(label);
    float tab_fontsize = TAB_LABEL_HEIGHT - 2.0f * TAB_LABEL_PADDING;
    Vector2 text_size = MeasureTextEx(font, label.c_str(), tab_fontsize, 3.0f);
    right += text_size.x + 2.0f * TAB_LABEL_PADDING + TAB_LABEL_MARGIN_LEFT;
  }

  void add_element_tab(int tab_idx, TextBox& textbox)
  {
    tabs[tab_idx].add_element(textbox);
  }

  void add_element_tab(int tab_idx, Button& button)
  {
    tabs[tab_idx].add_element(button);
  }

  void add_element_tab(int tab_idx, Switch& switch_)
  {
    tabs[tab_idx].add_element(switch_);
  }

  void receber_input(int key, char key_char)
  {
    tabs[selected_tab].receber_input(key, key_char);
  }

  void add_tab_objeto(Objeto* objeto, std::string label)
  {
    add_tab(label);
    tabs[selected_tab].add_object_controls(objeto, label);
  }

  void add_tab_luz(iluminacao::FontePontual* fonte, std::string label)
  {
    add_tab(label);
    tabs[selected_tab].add_light_controls(fonte, label);
  }
  void add_tab_luz(iluminacao::FonteDirecional* fonte, std::string label)
  {
    add_tab(label);
    tabs[selected_tab].add_light_controls(fonte, label);
  }
  void add_tab_luz(iluminacao::FonteSpot* fonte, std::string label)
  {
    add_tab(label);
    tabs[selected_tab].add_light_controls(fonte, label);
  }
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

double deltinhax, deltinhay;
int Deltax = W_C / nCol, Deltay = H_C / nLin;
Vetor3d Ponto_Superior_Esquerdo = { -W_J * 0.5f, W_J * 0.5f, -d };
float zp = -d;

// definicao da fonte luminosa
std::vector<iluminacao::FontePontual> fontes_pontuais;
std::vector<iluminacao::FonteDirecional> fontes_direcionais;
std::vector<iluminacao::FonteSpot> fontes_spot;

std::vector<std::string> fontes_pontuais_labels;
std::vector<std::string> fontes_direcionais_labels;
std::vector<std::string> fontes_spot_labels;

// definicao da iluminacao ambiente
Vetor3d I_A = { 1.0f, 1.0f, 1.0f };

std::vector<Objeto> objetos;

std::vector<std::string> objetos_labels;

void
inicializar_objetos()
{
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

  Vetor3d K_d_plano_esq = { 0.3f, 0.3f, 0.7f };
  Vetor3d K_e_plano_esq = { 0.0f, 0.0f, 0.0f };
  Vetor3d K_a_plano_esq = { 0.3f, 0.3f, 0.7f };
  float m_plano_esq = 1;
  Plano plano_esq({ -100.0f, 0.0f, 0.0f },
                  { 1.0f, 0.0f, 0.0f },
                  K_d_plano_esq,
                  K_e_plano_esq,
                  K_a_plano_esq,
                  m_plano_esq);

  Vetor3d K_d_plano_dir = { 0.3f, 0.3f, 0.7f };
  Vetor3d K_e_plano_dir = { 0.0f, 0.0f, 0.0f };
  Vetor3d K_a_plano_dir = { 0.3f, 0.3f, 0.7f };
  float m_plano_dir = 1;
  Plano plano_dir({ 100.0f, 0.0f, 0.0f },
                  { -1.0f, 0.0f, 0.0f },
                  K_d_plano_dir,
                  K_e_plano_dir,
                  K_a_plano_dir,
                  m_plano_dir);

  Malha cubo;
  Vetor3d K_cubo = { 0.7f, 0.3f, 0.3f };
  float m_cubo = 1.0f;
  cubo.inicializar_cubo(
    { 0.0f, 0.0f, -100.0f }, 30.0f, K_cubo, K_cubo, K_cubo, m_cubo);

  objetos.emplace_back(plano_fundo);
  objetos.emplace_back(plano_esq);
  objetos.emplace_back(plano_dir);
  objetos.emplace_back(cubo);

  objetos_labels.push_back("plano_fundo");
  objetos_labels.push_back("plano_esq");
  objetos_labels.push_back("plano_dir");
  objetos_labels.push_back("cubo");

  fontes_pontuais.push_back(
    iluminacao::FontePontual({ 0.0f, 60.0f, -30.0f }, { 0.7f, 0.7f, 0.7f }));
  fontes_pontuais_labels.push_back("luz_pontual");
}

RenderTexture2D tela;
bool ortografica = false;
Vetor3d Eye = { 0.0f, 0.0f, 0.0f };
Vetor3d At = { 0.0f, 0.0f, -20.0f };
Vetor3d Up = { 0.0f, 1.0f, 0.0f };
Camera3de camera(Eye, At, Up);

void
renderizar()
{
  TraceLog(LOG_INFO, "Renderizando");
  for (const iluminacao::FontePontual& fonte : fontes_pontuais) {
    TraceLog(LOG_INFO,
             "pontual.pos = (%f, %f, %f)",
             fonte.posicao.x,
             fonte.posicao.y,
             fonte.posicao.z);
  }

  Matriz M_cw = camera.getMatrixCameraWorld();

  Vetor3d PSE = (M_cw * camera.get_PSE().ponto4d()).vetor3d();
  Vetor3d right = { 1.0f, 0.0f, 0.0f };
  Vetor3d down = { 0.0f, -1.0f, 0.0f };
  Vetor3d forward = camera.get_center().normalizado();
  right = (M_cw * right.vetor4d()).vetor3d();
  down = (M_cw * down.vetor4d()).vetor3d();
  forward = (M_cw * forward.vetor4d()).vetor3d();

  deltinhax = camera.get_W_J() / nCol, deltinhay = camera.get_H_J() / nLin;

  BeginTextureMode(tela);
  {
    ClearBackground(BLACK);

    for (int i = 0; i < nLin; ++i) {
      for (int j = 0; j < nCol; ++j) {
        Vetor3d P = PSE + right * (deltinhax * (j + 0.5f)) +
                    down * (deltinhay * (i + 0.5f));
        Vetor3d dr;
        if (ortografica) {
          dr = forward;
        } else {
          dr = (P - camera.position).normalizado();
        }
        Raio raio(ortografica ? P : camera.position, dr);
        auto [t, objeto] = calcular_intersecao(raio, objetos);
        Vetor3d I_total = { 0.0f, 0.0f, 0.0f };
        Vetor3d Pt = raio.no_ponto(t);
        for (const iluminacao::FontePontual& fonte : fontes_pontuais) {
          if (!fonte.acesa)
            continue;
          Vetor3d dr_luz = fonte.posicao - Pt;
          float dist_luz = dr_luz.tamanho();
          if (dist_luz == 0.0f)
            continue;
          dr_luz = dr_luz * (1.0f / dist_luz);
          Raio raio_luz(Pt, dr_luz);
          auto [t_luz, _] = calcular_intersecao(raio_luz, objetos, objeto);
          if (t_luz < 0.0 || t_luz > dist_luz) {
            I_total =
              I_total + iluminacao::modelo_phong(Pt,
                                                 raio.dr,
                                                 objetos[objeto].normal(Pt),
                                                 fonte,
                                                 objetos[objeto].material);
          }
        }
        for (const iluminacao::FonteDirecional& fonte : fontes_direcionais) {
          if (!fonte.acesa)
            continue;
          Vetor3d dr_luz = fonte.direcao;
          Raio raio_luz(Pt, dr_luz);
          auto [t_luz, _] = calcular_intersecao(raio_luz, objetos, objeto);
          if (t_luz < 0.0) {
            I_total =
              I_total + iluminacao::modelo_phong(Pt,
                                                 raio.dr,
                                                 objetos[objeto].normal(Pt),
                                                 fonte,
                                                 objetos[objeto].material);
          }
        }
        for (const iluminacao::FonteSpot& fonte : fontes_spot) {
          if (!fonte.acesa)
            continue;
          Vetor3d dr_luz = fonte.posicao - Pt;
          float dist_luz = dr_luz.tamanho();
          dr_luz = dr_luz * (1.0f / dist_luz);
          Raio raio_luz(Pt, dr_luz);
          auto [t_luz, _] = calcular_intersecao(raio_luz, objetos, objeto);
          if (t_luz < 0.0 || t_luz > dist_luz) {
            I_total =
              I_total + iluminacao::modelo_phong(Pt,
                                                 raio.dr,
                                                 objetos[objeto].normal(Pt),
                                                 fonte,
                                                 objetos[objeto].material);
          }
        }

        I_total =
          I_total + iluminacao::luz_ambiente(I_A, objetos[objeto].material.K_a);

        DrawRectangle(
          Deltax * j,
          Deltay * i,
          Deltax,
          Deltay,
          (Color){ static_cast<unsigned char>(min(I_total.x * 255.0f, 255.0f)),
                   static_cast<unsigned char>(min(I_total.y * 255.0f, 255.0f)),
                   static_cast<unsigned char>(min(I_total.z * 255.0f, 255.0f)),
                   255 });
      }
    }
  }
  EndTextureMode();
  TraceLog(LOG_INFO, "Renderizacao completa");
}

int
main(void)
{

  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Interatividade");
  SetTargetFPS(60);

  Font font = GetFontDefault();

  int objeto_selecionado = -1;

  tela = LoadRenderTexture(W_C, H_C);

  inicializar_objetos();

  renderizar();

  TabbedPanel panel(
    { 500.0f, 0.0f, 300.0f, 450.0f }, font, []() { renderizar(); });
  panel.add_tab("camera");
  int camera_tab = panel.selected_tab;
  panel.tabs[camera_tab].add_camera_controls(&camera, "camera");
  panel.tabs[camera_tab].add_color_controls(&I_A, "I_amb");
  Switch switch_projecao(
    "Projecao Ortografica", { 0.0f, 0.0f, 30.0f, 15.0f }, &ortografica);
  panel.add_element_tab(camera_tab, switch_projecao);
  Rectangle btn_rect = { 0.0f, 0.0f, 260.0f, 30.0f };
  Button btn_camera("Atualizar Camera", btn_rect, [&panel, &camera_tab]() {
    for (TextBox& textbox : panel.tabs[camera_tab].textboxes)
      textbox.atualizar_parametro();
    camera.updateCoordinates();
  });
  panel.add_element_tab(camera_tab, btn_camera);
  Button btn_fuga1("1 Ponto de Fuga", btn_rect, [&panel, &camera_tab]() {
    camera.position = { 0.0f, 30.0f, 0.0f };
    camera.lookAt = { 0.0f, 30.0f, -20.0f };
    camera.Up = { 0.0f, 31.0f, 0.0f };
    camera.xmin = -30.0f;
    camera.ymin = -30.0f;
    camera.xmax = 30.0f;
    camera.ymax = 30.0f;
    ortografica = false;
    camera.updateCoordinates();
    for (TextBox& textbox : panel.tabs[camera_tab].textboxes)
      textbox.atualizar_texto();
  });
  panel.add_element_tab(camera_tab, btn_fuga1);
  Button btn_fuga2("2 Pontos de Fuga", btn_rect, [&panel, &camera_tab]() {
    camera.position = { 100.0f, 0.0f, 0.0f };
    camera.lookAt = { 0.0f, 0.0f, -100.0f };
    camera.Up = { 100.0f, 1.0f, 0.0f };
    camera.xmin = -30.0f;
    camera.ymin = -30.0f;
    camera.xmax = 30.0f;
    camera.ymax = 30.0f;
    ortografica = false;
    camera.updateCoordinates();
    for (TextBox& textbox : panel.tabs[camera_tab].textboxes)
      textbox.atualizar_texto();
  });
  panel.add_element_tab(camera_tab, btn_fuga2);
  Button btn_fuga3("3+ Pontos de Fuga", btn_rect, [&panel, &camera_tab]() {
    camera.position = { 60.0f, 50.0f, -40.0f };
    camera.lookAt = { 0.0f, 0.0f, -100.0f };
    camera.Up = { 60.0f, 51.0f, -40.0f };
    camera.xmin = -30.0f;
    camera.ymin = -30.0f;
    camera.xmax = 30.0f;
    camera.ymax = 30.0f;
    ortografica = false;
    camera.updateCoordinates();
    for (TextBox& textbox : panel.tabs[camera_tab].textboxes)
      textbox.atualizar_texto();
  });
  panel.add_element_tab(camera_tab, btn_fuga3);
  Button btn_obliqua("Obliqua", btn_rect, [&panel, &camera_tab]() {
    camera.position = { 50.0f, -30.0f, 0.0f };
    camera.lookAt = { 50.0f, -30.0f, -10.0f };
    camera.Up = { 50.0f, 0.0f, 0.0f };
    camera.xmin = -50.0f;
    camera.ymin = -20.0f;
    camera.xmax = 10.0f;
    camera.ymax = 40.0f;
    ortografica = true;
    camera.updateCoordinates();
    for (TextBox& textbox : panel.tabs[camera_tab].textboxes)
      textbox.atualizar_texto();
  });
  panel.add_element_tab(camera_tab, btn_obliqua);
  Button btn_zoom_in("Zoom in", btn_rect, [&panel, &camera_tab]() {
    if (ortografica) {
      Vetor3d centro = camera.get_center();
      float dx = (camera.xmax - centro.x) * 0.10f;
      float dy = (camera.ymax - centro.y) * 0.10f;
      camera.xmax -= dx;
      camera.xmin += dx;
      camera.ymax -= dy;
      camera.ymin += dy;
    } else {
      Vetor3d dv = (camera.lookAt - camera.position) * 0.40f;
      camera.position = camera.position + dv;
      camera.lookAt = camera.lookAt + dv;
    }
    camera.updateCoordinates();
    for (TextBox& textbox : panel.tabs[camera_tab].textboxes)
      textbox.atualizar_texto();
  });
  panel.add_element_tab(camera_tab, btn_zoom_in);
  Button btn_zoom_out("Zoom out", btn_rect, [&panel, &camera_tab]() {
    if (ortografica) {
      Vetor3d centro = camera.get_center();
      float dx = (camera.xmax - centro.x) * 0.10f;
      float dy = (camera.ymax - centro.y) * 0.10f;
      camera.xmax += dx;
      camera.xmin -= dx;
      camera.ymax += dy;
      camera.ymin -= dy;
    } else {
      Vetor3d dv = (camera.lookAt - camera.position) * 0.40f;
      camera.position = camera.position - dv;
      camera.lookAt = camera.lookAt - dv;
    }
    camera.updateCoordinates();
    for (TextBox& textbox : panel.tabs[camera_tab].textboxes)
      textbox.atualizar_texto();
  });
  panel.add_element_tab(camera_tab, btn_zoom_out);
  for (TextBox& textbox : panel.tabs[camera_tab].textboxes)
    textbox.atualizar_texto();

  for (size_t i = 0; i < fontes_pontuais.size(); ++i) {
    panel.add_tab_luz(&fontes_pontuais[i], fontes_pontuais_labels[i]);
  }

  for (size_t i = 0; i < fontes_direcionais.size(); ++i) {
    panel.add_tab_luz(&fontes_direcionais[i], fontes_direcionais_labels[i]);
  }

  for (size_t i = 0; i < fontes_spot.size(); ++i) {
    panel.add_tab_luz(&fontes_spot[i], fontes_spot_labels[i]);
  }

  while (!WindowShouldClose()) {

    Matriz M_cw = camera.getMatrixCameraWorld();

    Vetor3d PSE = (M_cw * camera.get_PSE().ponto4d()).vetor3d();
    Vetor3d right = { 1.0f, 0.0f, 0.0f };
    Vetor3d down = { 0.0f, -1.0f, 0.0f };
    Vetor3d forward = camera.get_center().normalizado();
    right = (M_cw * right.vetor4d()).vetor3d();
    down = (M_cw * down.vetor4d()).vetor3d();
    forward = (M_cw * forward.vetor4d()).vetor3d();

    deltinhax = camera.get_W_J() / nCol, deltinhay = camera.get_H_J() / nLin;

    Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      if (mouse.x < W_C && mouse.y < H_C) {
        int x_pos = mouse.x, y_pos = mouse.y;
        Vetor3d P = PSE + right * (deltinhax * (x_pos + 0.5f)) +
                    down * (deltinhay * (y_pos + 0.5f));
        Vetor3d dr;
        if (ortografica) {
          dr = forward;
        } else {
          dr = (P - camera.position).normalizado();
        }
        Raio raio(ortografica ? P : camera.position, dr);
        auto [t, objeto] = calcular_intersecao(raio, objetos);
        if (t > 0.0f)
          objeto_selecionado = objeto;
        TraceLog(LOG_INFO, "objeto_selecionado = %d", objeto_selecionado);
        if (objeto_selecionado >= 0)
          panel.add_tab_objeto(&objetos[objeto], objetos_labels[objeto]);
      } else {
        panel.intersecao(mouse);
      }
    }

    if (IsKeyDown(KEY_UP)) {
      panel.scroll_up();
    } else if (IsKeyDown(KEY_DOWN)) {
      panel.scroll_down();
    } else if (IsKeyDown(KEY_RIGHT)) {
      panel.scroll_tabs_right();
    } else if (IsKeyDown(KEY_LEFT)) {
      panel.scroll_tabs_left();
    }

    int key = GetKeyPressed();
    char key_char = GetCharPressed();
    panel.receber_input(key, key_char);

    BeginDrawing();
    {
      ClearBackground(BACKGROUND_COLOR);
      panel.desenhar();
      DrawTextureRec(
        tela.texture, { 0.0f, 0.0f, W_C, -H_C }, { 0.0f, 0.0f }, WHITE);
    }
    EndDrawing();
  }

  UnloadRenderTexture(tela);

  return 0;
}
