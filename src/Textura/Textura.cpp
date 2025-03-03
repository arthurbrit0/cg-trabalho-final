#include "Textura.h"
#include <math.h>

Textura::Textura(Color* pixels,
                 int col,
                 int lin,
                 float width,
                 float height,
                 float m)
  : pixels(pixels)
  , col(col)
  , lin(lin)
  , width(width)
  , height(height)
  , m(m)
{
}

Color
Textura::at(float x, float y)
{
  int pos_x = ((int)roundf(x / width * col) % col + col) % col;
  int pos_y = ((int)roundf(y / height * lin) % lin + lin) % lin;

  return pixels[pos_y * col + pos_x];
}
