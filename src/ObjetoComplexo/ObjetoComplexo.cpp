#include "ObjetoComplexo.h"

void
ObjetoComplexo::transformar(Matriz mat)
{
  for (Objeto& obj : objetos) {
    obj.transformar(mat);
  }

  for (ObjetoComplexo& obj_complexo : objetosComplexos) {
    obj_complexo.transformar(mat);
  }
}

void
ObjetoComplexo::adicionar_objeto(const Objeto& obj)
{
  objetos.push_back(obj);
}


void
ObjetoComplexo::adicionar_objeto_complexo(const ObjetoComplexo& obj_complexo)
{
  for (const Objeto& obj : obj_complexo.objetos) {
    objetos.push_back(obj);
  }
  for (const ObjetoComplexo& sub_complexo : obj_complexo.objetosComplexos) {
    objetosComplexos.push_back(sub_complexo);
  }
}