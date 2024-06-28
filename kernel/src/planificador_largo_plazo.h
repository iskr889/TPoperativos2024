#ifndef PLANIFICADORLARGOPLAZO_H_
#define PLANIFICADORLARGOPLAZO_H_

#include "main.h"

void planificador_largo_plazo();
void *planificador_largo_new_a_ready();
void cambiar_grado_multiprogramacion(int nuevo_grado_multi);
void esperar_confirmacion_memoria();

#endif