#ifndef SERVERS_CPU_H
#define SERVERS_CPU_H

#include "instruccion.h"
#include "../../utils/src/pcb.h"

// typedef struct {
//     uint32_t direccion_fisica;
//     bool valido;
// } traduccion_t;



void iniciar_dispatch();
void iniciar_interrupt();
void ciclo_instruccion(pcb_t* pcb);

//traduccion_t traducir_direccion(uint32_t direccion_logica);
void enviar_pc(uint32_t pc);
#endif