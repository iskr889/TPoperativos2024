#include "servers.h"
int socket_memoria;

void iniciar_dispatch() {
    int socket_server = escuchar_conexiones_de("KERNEL (PUERTO DISPATCH)", config.puerto_escucha_dispath, logger);

    if (socket_server < 0) {
        log_error(logger, "NO SE PUDO INICIAR EL SERVIDOR PARA QUE OTROS MODULOS SE CONECTEN");
        exit(EXIT_FAILURE);
    }
    socket_memoria = conectarse_a_modulo("MEMORIA", config.ip_memoria, config.puerto_memoria, CPU_CON_MEMORIA, logger);
    log_info(logger, "SERVIDOR INICIADO... ESPERANDO LA CONEXION DE KERNEL");
    int socket_kernel = esperar_conexion_de(KERNEL_CON_CPU_DISPATCH, socket_server);
    log_info(logger, "[CPU] CONEXION ESTABLECIDA CON EL KERNEL");
    
    while(1) {
        pcb_t* pcb = receive_pcb(socket_kernel);
        printf("tesT\n");
        ciclo_instruccion(pcb);
        sleep(1);
        free(pcb);
    }

    close(socket_kernel);
    close(socket_server);

}
void iniciar_interrupt() {
    int socket_server = escuchar_conexiones_de("KERNEL (PUERTO INTERRUPT)", config.puerto_escucha_interrupt, logger);
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