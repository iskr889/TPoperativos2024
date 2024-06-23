#ifndef SERVERS_CPU_H
#define SERVERS_CPU_H

#include "main.h"

// typedef struct {
//     uint32_t direccion_fisica;
//     bool valido;
// } traduccion_t;

void* iniciar_dispatch();
void* iniciar_interrupt();
void ciclo_instruccion(pcb_t* pcb);
void enviar_interrupcion_FINALIZADO(int conexion_interrupt, pcb_t *pcb);

#endif