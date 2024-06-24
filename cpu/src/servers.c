#include "servers.h"
#include "instruccion.h"

extern t_cpu_config cpu_config;
extern t_log* logger;
extern t_log* extra_logger;
extern int conexion_dispatch, conexion_interrupt;

pthread_mutex_t mutex_pcb_estado = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_pcb_exit = PTHREAD_COND_INITIALIZER;
pcb_t* current_pcb = NULL;

void* iniciar_dispatch() {
    log_debug(extra_logger, "INICIO HILO DISPATCH");

    while(1) {

        pcb_t* pcb = receive_pcb(conexion_dispatch);

        sleep(1);

        ciclo_instruccion(pcb);
    }

    return NULL;
}

void* iniciar_interrupt() {
    log_debug(extra_logger, "INICIO HILO INTERRUPT");

    while(1) {

        pthread_mutex_lock(&mutex_pcb_estado);

        // Esperar que pcb->estado == EXIT para enviar interrupción al kernel
        while (current_pcb == NULL || current_pcb->estado != EXIT) {
            pthread_cond_wait(&cond_pcb_exit, &mutex_pcb_estado);
        }

        log_debug(extra_logger, "PCB con PID %d ha terminado en EXIT", current_pcb->pid);

        enviar_interrupcion_FINALIZADO(conexion_interrupt, current_pcb);

        free(current_pcb);
        current_pcb = NULL;

        pthread_mutex_unlock(&mutex_pcb_estado);
    }

    return NULL;
}

void ciclo_instruccion(pcb_t* pcb) {
    char* instruccion;
    imprimir_pcb(pcb);

    pthread_mutex_lock(&mutex_pcb_estado);
    current_pcb = pcb;
    pthread_mutex_unlock(&mutex_pcb_estado);

    while ((instruccion = fetch(pcb)) != NULL) {

        char *instruccion_deco = decode(instruccion, pcb);
        execute(instruccion_deco, pcb);
        free(instruccion);

        imprimir_pcb(pcb);

        if (pcb->estado == EXIT) {
            pthread_mutex_lock(&mutex_pcb_estado);
            pthread_cond_signal(&cond_pcb_exit);
            pthread_mutex_unlock(&mutex_pcb_estado);
            break;
        }
    }
}

void enviar_interrupcion_FINALIZADO(int conexion_interrupt, pcb_t* pcb) {
    log_debug(extra_logger, "Enviando interrupción al kernel");
    enviar_interrupcion(conexion_interrupt, pcb, "EXIT", FINALIZADO);
}