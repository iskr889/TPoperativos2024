#ifndef MANEJARINSTRUCCIONES_H_
#define MANEJARINSTRUCCIONES_H_

#include "main.h"
#include "../../utils/src/serializacion.h"

#define CARPETA_PSEUDOCODIGO "pseudocodigo/"

void *thread_instrucciones_kernel(void *);

void manejar_instrucciones_kernel();

void instruccion_process_create(payload_t* payload);

t_list *leer_pseudocodigo(String filename);

void imprimir_instrucciones(t_list* instrucciones);

void imprimir_instruccion(String instruccion);

void imprimir_instruccion_numero(t_list* instrucciones, uint32_t numero_de_linea);

#endif