#ifndef SERVERS_CPU_H
#define SERVERS_CPU_H

#include "main.h"

void* hilo_ciclo_instruccion();
void ciclo_instruccion(pcb_t* pcb);
void* hilo_interrupcion();
void responder_interrupcion(pcb_t *pcb, int codigo_operacion);

#endif