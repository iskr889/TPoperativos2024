#ifndef RECURSOS_H_
#define RECURSOS_H_

#include "main.h"

void init_recursos();

void restar_recurso(t_dictionary *diccionario, char* recurso);

void sumar_recurso(t_dictionary *diccionario, char* recurso);

void imprimir_instancia_prueba(t_dictionary *diccionario, char* recurso);

int obtenerInstancia(t_dictionary *diccionario, char *recurso);

#endif