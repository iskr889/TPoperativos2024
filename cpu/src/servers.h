#ifndef SERVERS_CPU_H
#define SERVERS_CPU_H

#include "main.h"

void* iniciar_dispatch();
void* iniciar_interrupt();
void ciclo_instruccion(pcb_t* pcb);

#endif