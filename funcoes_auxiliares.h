// Created by Vitor de Albuquerque on 25/10/2024.

#ifndef FUNCOES_AUXILIARES_H
#define FUNCOES_AUXILIARES_H

#define DIMENSAO_MAXIMA 4

namespace funcoes_auxiliares {

struct Matriz;
struct Vetor3d;

struct Matriz
{
  int lin;
  int col;

  float data[DIMENSAO_MAXIMA][DIMENSAO_MAXIMA];

  Matriz transposta();
  Vetor3d vetor3d();

  static Matriz identidade(int dimensao);
  static Matriz nula(int lin, int col);
  static Matriz translacao(Vetor3d vetor_translacao);
  static Matriz escala(Vetor3d vetor_escala);
  static Matriz escala(Vetor3d vetor_escala, Vetor3d ponto_fixo);
  static Matriz rotacao_x(float angulo);
  static Matriz rotacao_y(float angulo);
  static Matriz rotacao_z(float angulo);
  static Matriz rotacao_eixo(Vetor3d eixo, float angulo);
  static Matriz cisalhamento_xy_x(float angulo);
  static Matriz cisalhamento_xy_y(float angulo);
  static Matriz cisalhamento_xz_x(float angulo);
  static Matriz cisalhamento_xz_z(float angulo);
  static Matriz cisalhamento_yz_y(float angulo);
  static Matriz cisalhamento_yz_z(float angulo);
  static Matriz espelhamento_xy();
  static Matriz espelhamento_xz();
  static Matriz espelhamento_yz();
  static Matriz espelhamento(Vetor3d normal);
  static Matriz espelhamento(Vetor3d normal, Vetor3d ponto);
};
// Funções max e min
double
max(double a, double b);
double
min(double a, double b);

// Definição da estrutura Vetor3d
struct Vetor3d
{
  float x; // Componente x
  float y; // Componente y
  float z; // Componente z

  float dot_product(Vetor3d vetor);
  Vetor3d cross_product(Vetor3d vetor);
  float tamanho();
  Vetor3d normalizado();
  Matriz matriz();
  Matriz ponto4d();
  Matriz vetor4d();
};
// Métodos auxiliares de operação
Vetor3d
operator*(Vetor3d vetor, float escalar);
Vetor3d
operator*(float escalar, Vetor3d vetor);
Vetor3d
operator*(Vetor3d vetor1, Vetor3d vetor2);
Vetor3d
operator+(Vetor3d vetor1, Vetor3d vetor2);
Vetor3d
operator-(Vetor3d vetor1, Vetor3d vetor2);

Matriz
operator+(Matriz matriz1, Matriz matriz2);

Matriz
operator-(Matriz matriz1, Matriz matriz2);

Matriz
operator*(Matriz matriz1, Matriz matriz2);

Matriz
operator*(float escalar, Matriz matriz);

Matriz
operator*(Matriz matriz, float escalar);

Matriz
operator*(Vetor3d vetor, Matriz matriz);

Matriz
operator*(Matriz matriz, Vetor3d vetor);

}

#endif // FUNCOES_AUXILIARES_H
