#include "servers.h"
#include "instruccion.h"

extern t_cpu_config cpu_config;
extern t_log* logger;
extern t_log* extra_logger;
extern int conexion_dispatch, conexion_interrupt;

void* iniciar_dispatch() {
    log_debug(extra_logger, "INICIO HILO DISPATCH");
    while(1) {
        pcb_t* pcb = receive_pcb(conexion_dispatch);
        ciclo_instruccion(pcb);
        sleep(1);
        free(pcb);
    }

    return NULL;
}

void* iniciar_interrupt() {
    log_debug(extra_logger, "INICIO HILO INTERRUPT");
    while(1) {
        // Esperar interrupción
    }

    return NULL;
}

void ciclo_instruccion(pcb_t* pcb) {
    char* instruccion;
    imprimir_pcb(pcb);
    printf("---INIT---");
    while ((instruccion = fetch(pcb)) != NULL) {
        decode(instruccion, pcb);
        execute(instruccion, pcb);
        free(instruccion);
        imprimir_pcb(pcb);
        printf("---END ITERACION---");
        if (pcb->estado == EXIT) {
            break;
        }
    }

    printf("END CICLO\n");
    sleep(1);
}