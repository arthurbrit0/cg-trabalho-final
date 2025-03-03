#ifndef INICIALIZAR_OBJETOS_H
#define INICIALIZAR_OBJETOS_H

#include <vector>
#include "./Objeto/Objeto.h"
#include "./ObjetoComplexo/ObjetoComplexo.h"

void inicializar_objetos(std::vector<Objeto>& objects_flat, std::vector<ObjetoComplexo>& complexObjects);

void flatten_objetos(ObjetoComplexo& objeto_complexo, std::vector<Objeto>& objetos_flat);

void  deletar_objetos(Color *pixels_textura, Image textura);

#endif // INICIALIZAR_OBJETOS_H