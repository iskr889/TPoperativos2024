#include "servers.h"
#include "instruccion.h"

#define CLEAR_INTERRUPT 0

extern t_cpu_config cpu_config;
extern t_log* logger;
extern t_log* extra_logger;
extern int conexion_dispatch, conexion_interrupt;
volatile int interrupcion = 0; // Agrego esta variable global, puede tomar el estado de FINALIZADO o DESALOJO_QUANTUM

void* hilo_ciclo_instruccion() {

    log_debug(extra_logger, "INICIÓ EL CICLO DE INSTRUCCIÓN");

    while(1) {

        pcb_t* pcb = receive_pcb(conexion_dispatch);

        imprimir_pcb(pcb);

        ciclo_instruccion(pcb);

        free(pcb);
    }

    return NULL;
}

void ciclo_instruccion(pcb_t* pcb) {

    interrupcion = CLEAR_INTERRUPT;

    char* instruction;

    while ((instruction = fetch(pcb)) != NULL) {

        char *decoded_instruction = decode(instruction, pcb);

        execute(decoded_instruction, pcb);

        imprimir_pcb(pcb);

        if (pcb->estado == EXIT)
            break;

        if (pcb->estado == BLOCKED)
            break;

        if (interrupcion == FINALIZADO) {
            pcb->estado = EXIT;
            responder_interrupcion(pcb, FINALIZADO);
            log_debug(extra_logger, "INTERRUPCIÓN RECIBIDA: FINALIZADO");
            break;
        }

        if (interrupcion == DESALOJO_QUANTUM) {
            responder_interrupcion(pcb, DESALOJO_QUANTUM);
            log_debug(extra_logger, "INTERRUPCIÓN RECIBIDA: DESALOJO_QUANTUM");
            break;
        }
    }
}

void *hilo_interrupcion() {
    while(1) {
        interrupcion = recibir_operacion(conexion_interrupt);
        if(interrupcion < 0)
            pthread_exit(0);
    }
}

void responder_interrupcion(pcb_t *pcb, int codigo_operacion) {
    enviar_contexto(conexion_dispatch, pcb, " ", codigo_operacion);
}
