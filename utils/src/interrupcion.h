#ifndef INTERRUPCION_H_
#define INTERRUPCION_H_

#include "pcb.h"


typedef struct{
    pcb_t *pcb;
    String instruccion;
} interrupcion_t;


payload_t *interrupcion_serializar(pcb_t *pcb, String instruccion);
interrupcion_t *interrupcion_deserializar(payload_t *payload);
void enviar_interrupcion(int socket, pcb_t *pcb, String instruccion, int codigo_operacion);
paquete_t *recibir_interrupcion(int socket);

#endif