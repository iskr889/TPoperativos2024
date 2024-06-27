#include "servers.h"
#include "instruccion.h"

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

        sleep(1); // Espera que se cree el pcb en la memoria, buscar una mejor solución

        ciclo_instruccion(pcb);

        free(pcb);
    }

    return NULL;
}

void ciclo_instruccion(pcb_t* pcb) {

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
            interrupcion = OK;
            pcb->estado = EXIT;
            enviar_interrupcion(conexion_dispatch, pcb, "EXIT", FINALIZADO);
            break;
        }

        if (interrupcion == DESALOJO_QUANTUM) {
            interrupcion = OK;
            enviar_interrupcion(conexion_dispatch, pcb, "DESALOJO_QUANTUM", DESALOJO_QUANTUM);
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
