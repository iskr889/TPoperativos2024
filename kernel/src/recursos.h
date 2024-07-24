#ifndef RECURSOS_H_
#define RECURSOS_H_

#include "main.h"
#include "scheduler.h"

void init_recursos();

void restar_recurso(t_dictionary *diccionario, char* recurso);

void sumar_recurso(t_dictionary *diccionario, char* recurso);

void imprimir_instancia_prueba(t_dictionary *diccionario, char* recurso);

int obtenerInstancia(t_dictionary *diccionario, char *recurso);

void asignar_recurso_a_proceso(int pid, char *recurso);

void liberar_recurso_de_proceso(int pid, char *recurso);

void liberar_recursos_de_proceso(int pid);

void liberar_recurso_aux(char *key, void *instancia);

void sumar_recursos(t_dictionary *diccionario, char* recurso, int cantidad);

void inicializar_recursos_proceso(int pid);

void destruir_diccionario_recursos_asignados();

void inicializar_lista_recursos_asignados(t_dictionary *diccionario);

#endif
