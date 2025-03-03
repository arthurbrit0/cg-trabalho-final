#ifndef TEXTURA_H
#define TEXTURA_H

#include <raylib.h>

struct Textura
{
  int col, lin;        // tamanho em pixels
  float width, height; // tamanho no cena
  float m;
  Color* pixels;

  Textura(Color* pixels, int col, int lin, float width, float height, float m);

  Color at(float x, float y);
};

#endif // TEXTURA_H
