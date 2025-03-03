// Created by Vitor de Albuquerque on 25/10/2024.

#include "funcoes_auxiliares.h"
#include <cassert>
#include <cmath> // Para sqrtf()

namespace funcoes_auxiliares {
// Funções max e min

double
max(double a, double b)
{
  return a + (b - a) * (a < b);
}

double
min(double a, double b)
{
  return a + (b - a) * (a > b);
}
float
Vetor3d::dot_product(Vetor3d vetor)
{
  return x * vetor.x + y * vetor.y + z * vetor.z;
}

Vetor3d
Vetor3d::cross_product(Vetor3d vetor)
{
  Vetor3d resultado = { y * vetor.z - z * vetor.y,
                        z * vetor.x - x * vetor.z,
                        x * vetor.y - y * vetor.x };
  return resultado;
}

float
Vetor3d::tamanho()
{
  return sqrtf(x * x + y * y + z * z);
}

Vetor3d
Vetor3d::normalizado()
{
  float tam = tamanho();
  Vetor3d resultado = { x / tam, y / tam, z / tam };
  return resultado;
}

Matriz
Vetor3d::matriz()
{
  Matriz res = { .lin = 3, .col = 1 };
  res.data[0][0] = x;
  res.data[1][0] = y;
  res.data[2][0] = z;
  return res;
}

Matriz
Vetor3d::ponto4d()
{
  Matriz res = { .lin = 4, .col = 1 };
  res.data[0][0] = x;
  res.data[1][0] = y;
  res.data[2][0] = z;
  res.data[3][0] = 1.0f;
  return res;
}

Matriz
Vetor3d::vetor4d()
{
  Matriz res = { .lin = 4, .col = 1 };
  res.data[0][0] = x;
  res.data[1][0] = y;
  res.data[2][0] = z;
  res.data[3][0] = 0.0f;
  return res;
}

Vetor3d
operator*(Vetor3d vetor, float escalar)
{
  Vetor3d resultado = { vetor.x * escalar,
                        vetor.y * escalar,
                        vetor.z * escalar };
  return resultado;
}

Vetor3d
operator*(float escalar, Vetor3d vetor)
{
  Vetor3d resultado = { vetor.x * escalar,
                        vetor.y * escalar,
                        vetor.z * escalar };
  return resultado;
}

Vetor3d
operator*(Vetor3d vetor1, Vetor3d vetor2)
{
  Vetor3d resultado = { vetor1.x * vetor2.x,
                        vetor1.y * vetor2.y,
                        vetor1.z * vetor2.z };
  return resultado;
}

Vetor3d
operator+(Vetor3d vetor1, Vetor3d vetor2)
{
  Vetor3d resultado = { vetor1.x + vetor2.x,
                        vetor1.y + vetor2.y,
                        vetor1.z + vetor2.z };
  return resultado;
}

Vetor3d
operator-(Vetor3d vetor1, Vetor3d vetor2)
{
  Vetor3d resultado = { vetor1.x - vetor2.x,
                        vetor1.y - vetor2.y,
                        vetor1.z - vetor2.z };
  return resultado;
}

Matriz
Matriz::transposta()
{
  Matriz res = { .lin = col, .col = lin };
  for (int i = 0; i < lin; i++) {
    for (int j = 0; j < col; ++j) {
      res.data[j][i] = data[i][j];
    }
  }
  return res;
}

Vetor3d
Matriz::vetor3d()
{
  assert(col == 1);
  assert(lin >= 3);
  return (Vetor3d){ .x = data[0][0], .y = data[1][0], .z = data[2][0] };
}

Matriz
Matriz::identidade(int dimensao)
{
  Matriz res = { .lin = dimensao, .col = dimensao, .data = { 0 } };
  for (int i = 0; i < dimensao; ++i) {
    res.data[i][i] = 1.0f;
  }
  return res;
}

Matriz
Matriz::nula(int lin, int col)
{
  return { .lin = lin, .col = col, .data = { 0 } };
}

Matriz
Matriz::translacao(Vetor3d vetor_translacao)
{
  Matriz res = Matriz::identidade(4);
  res.data[0][3] = vetor_translacao.x;
  res.data[1][3] = vetor_translacao.y;
  res.data[2][3] = vetor_translacao.z;
  return res;
}

Matriz
Matriz::escala(Vetor3d vetor_escala)
{
  Matriz res = Matriz::nula(4, 4);
  res.data[0][0] = vetor_escala.x;
  res.data[1][1] = vetor_escala.y;
  res.data[2][2] = vetor_escala.z;
  res.data[3][3] = 1.0f;
  return res;
}

Matriz
Matriz::escala(Vetor3d vetor_escala, Vetor3d ponto_fixo)
{
  Vetor3d origem = { 0.0f, 0.0f, 0.0f };
  return Matriz::translacao(ponto_fixo - origem) *
         Matriz::escala(vetor_escala) * Matriz::translacao(origem - ponto_fixo);
}

Matriz
Matriz::rotacao_x(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[1][1] = cos(angulo);
  res.data[1][2] = -sin(angulo);
  res.data[2][1] = sin(angulo);
  res.data[2][2] = cos(angulo);
  return res;
}

Matriz
Matriz::rotacao_y(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[0][0] = cos(angulo);
  res.data[2][0] = -sin(angulo);
  res.data[0][2] = sin(angulo);
  res.data[2][2] = cos(angulo);
  return res;
}

Matriz
Matriz::rotacao_z(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[0][0] = cos(angulo);
  res.data[0][1] = -sin(angulo);
  res.data[1][0] = sin(angulo);
  res.data[1][1] = cos(angulo);
  return res;
}

// eixo deve estar centrado na origem
Matriz
Matriz::rotacao_eixo(Vetor3d eixo, float angulo)
{
  Vetor3d u = eixo.normalizado();
  float seno = sin(0.5f * angulo), cosseno = cos(0.5f * angulo);
  float x = seno * u.x, y = seno * u.y, z = seno * u.z, w = cosseno;
  Matriz res = Matriz::identidade(4);
  res.data[0][0] = w * w + x * x - y * y - z * z;
  res.data[0][1] = 2 * (x * y - w * z);
  res.data[0][2] = 2 * (x * z + w * y);

  res.data[1][0] = 2 * (x * y + w * z);
  res.data[1][1] = w * w - x * x + y * y - z * z;
  res.data[1][2] = 2 * (y * z - w * x);

  res.data[2][0] = 2 * (x * z - w * y);
  res.data[2][1] = 2 * (y * z + w * x);
  res.data[2][2] = w * w - x * x - y * y + z * z;

  return res;
}

Matriz
Matriz::cisalhamento_xy_x(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[0][1] = tan(angulo);
  return res;
}

Matriz
Matriz::cisalhamento_xy_y(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[1][0] = tan(angulo);
  return res;
}

Matriz
Matriz::cisalhamento_xz_x(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[0][2] = tan(angulo);
  return res;
}

Matriz
Matriz::cisalhamento_xz_z(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[2][0] = tan(angulo);
  return res;
}

Matriz
Matriz::cisalhamento_yz_y(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[1][2] = tan(angulo);
  return res;
}

Matriz
Matriz::cisalhamento_yz_z(float angulo)
{
  Matriz res = Matriz::identidade(4);
  res.data[2][1] = tan(angulo);
  return res;
}

Matriz
Matriz::espelhamento_xy()
{
  Matriz res = Matriz::identidade(4);
  res.data[2][2] *= -1;
  return res;
}

Matriz
Matriz::espelhamento_xz()
{
  Matriz res = Matriz::identidade(4);
  res.data[1][1] *= -1;
  return res;
}

Matriz
Matriz::espelhamento_yz()
{
  Matriz res = Matriz::identidade(4);
  res.data[0][0] *= -1;
  return res;
}

Matriz
Matriz::espelhamento(Vetor3d normal)
{
  Matriz n = normal.matriz();
  return Matriz::identidade(4) - 2 * n * n.transposta();
}

Matriz
Matriz::espelhamento(Vetor3d normal, Vetor3d ponto)
{
  Vetor3d origem = { 0.0f, 0.0f, 0.0f };
  return Matriz::translacao(ponto - origem) * Matriz::espelhamento(normal) *
         Matriz::translacao(origem);
}

Matriz
operator+(Matriz matriz1, Matriz matriz2)
{
  assert(matriz1.lin == matriz2.lin && matriz1.col == matriz2.col);
  Matriz res = { .lin = matriz1.lin, .col = matriz1.col };
  for (int i = 0; i < res.lin; ++i) {
    for (int j = 0; j < res.col; ++j) {
      res.data[i][j] = matriz1.data[i][j] + matriz2.data[i][j];
    }
  }
  return res;
}

Matriz
operator-(Matriz matriz1, Matriz matriz2)
{
  assert(matriz1.lin == matriz2.lin && matriz1.col == matriz2.col);
  Matriz res = { .lin = matriz1.lin, .col = matriz1.col };
  for (int i = 0; i < res.lin; ++i) {
    for (int j = 0; j < res.col; ++j) {
      res.data[i][j] = matriz1.data[i][j] - matriz2.data[i][j];
    }
  }
  return res;
}

Matriz
operator*(Matriz matriz1, Matriz matriz2)
{
  assert(matriz1.col == matriz2.lin);
  Matriz res = { .lin = matriz1.lin, .col = matriz2.col, .data = { 0 } };
  for (int i = 0; i < res.lin; ++i) {
    for (int j = 0; j < res.col; ++j) {
      for (int k = 0; k < matriz1.col; ++k) {
        res.data[i][j] += matriz1.data[i][k] * matriz2.data[k][j];
      }
    }
  }
  return res;
}

Matriz
operator*(float escalar, Matriz matriz)
{
  Matriz res = { .lin = matriz.lin, .col = matriz.col };
  for (int i = 0; i < res.lin; ++i) {
    for (int j = 0; j < res.col; ++j) {
      res.data[i][j] = escalar * matriz.data[i][j];
    }
  }
  return res;
}

Matriz
operator*(Matriz matriz, float escalar)
{
  return escalar * matriz;
}

}
