#include "funcoes_auxiliares.h"

#ifndef Camera_H
#define Camera_H

const float DEFAULT_D = 30.0f;
const float DEFAULT_XMIN = -30.0f;
const float DEFAULT_YMIN = -30.0f;
const float DEFAULT_XMAX = 30.0f;
const float DEFAULT_YMAX = 30.0f;

using namespace funcoes_auxiliares;
class Camera3de
{
public:
  Vetor3d position;
  Vetor3d lookAt;
  Vetor3d Up;

  float d;
  float xmin, ymin, xmax, ymax;

  Vetor3d i{}; // Vetor horizontal
  Vetor3d j{}; // Vetor vertical
  Vetor3d k{}; // Vetor direção para trás

  Camera3de(Vetor3d position, Vetor3d lookAt, Vetor3d Up);
  Camera3de(Vetor3d position,
            Vetor3d lookAt,
            Vetor3d Up,
            float d,
            float xmin,
            float ymin,
            float xmax,
            float ymax);

  void updateCoordinates();

  Matriz getTransformationMatrix();
  Matriz getMatrixCameraWorld();

  Vetor3d get_PSE();
  Vetor3d get_center();
  float get_W_J();
  float get_H_J();
};

#endif