#ifndef INTERRUPCION_H_
#define INTERRUPCION_H_

#include "pcb.h"

typedef struct{
    pcb_t *pcb;
    String instruccion;
} contexto_t;

payload_t *contexto_serializar(pcb_t *pcb, String instruccion);
contexto_t *contexto_deserializar(payload_t *payload);
void enviar_contexto(int socket, pcb_t *pcb, String instruccion, int codigo_operacion);
paquete_t *recibir_contexto(int socket);

#endif