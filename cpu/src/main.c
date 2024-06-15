#include "main.h"
#include "servers.h"

t_config* config;
t_cpu_config cpu_config;
t_log* logger;
t_log* extra_logger;
int conexion_memoria, dispatch_server, interrupt_server, conexion_dispatch, conexion_interrupt;

int main(int argc, char* argv[]) {

    load_cpu_config("cpu.config");

    logger = iniciar_logger("cpu.log", "CPU", 1, LOG_LEVEL_INFO);

    extra_logger = iniciar_logger("cpu_debug.log", "CPU", 1, LOG_LEVEL_DEBUG);

    // El Kernel intenta conectarse con la memoria
    conexion_memoria = conectarse_a_modulo("MEMORIA", cpu_config.ip_memoria, cpu_config.puerto_memoria, CPU_CON_MEMORIA, extra_logger);

    // La CPU inicia un servidor que escucha por conexiones del Kernel a la CPU (DISPATCH)
    dispatch_server = escuchar_conexiones_de("KERNEL (PUERTO DISPATCH)", cpu_config.puerto_escucha_dispath, extra_logger);

    // La CPU inicia un servidor que escucha por conexiones del Kernel a la CPU (INTERRUPT)
    interrupt_server = escuchar_conexiones_de("KERNEL (PUERTO INTERRUPT)", cpu_config.puerto_escucha_interrupt, extra_logger);

    // La CPU espera que el KERNEL se conecte al puerto Dispatch
    conexion_dispatch = esperar_conexion_de(KERNEL_CON_CPU_DISPATCH, dispatch_server);

    if(conexion_dispatch < 0) {
        log_error(extra_logger, "MODULO KERNEL NO PUDO CONECTARSE CON CPU DISPATCH!");
        exit(EXIT_FAILURE);
    }

    log_debug(extra_logger, "MODULO KERNEL CONECTO CON CPU DISPATCH EXITOSAMENTE!");

    // La CPU espera que el KERNEL se conecte al puerto Inrerrupt
    conexion_interrupt = esperar_conexion_de(KERNEL_CON_CPU_INTERRUPT, interrupt_server);

    if(conexion_interrupt < 0) {
        log_error(extra_logger, "MODULO KERNEL NO PUDO CONECTARSE CON CPU INTERRUPT!");
        exit(EXIT_FAILURE);
    }

    log_debug(extra_logger, "MODULO KERNEL CONECTO CON CPU INTERRUPT EXITOSAMENTE!");

    // Creo los hilos de dispatch e interrupt para manejar la comunicación con el kernel
    pthread_t thread_dispatch, thread_interrupt;
    pthread_create(&thread_dispatch, NULL, iniciar_dispatch, NULL);
    pthread_detach(thread_dispatch);
    pthread_create(&thread_interrupt, NULL, iniciar_interrupt, NULL);
    pthread_detach(thread_interrupt);
    pthread_exit(0);

    liberar_cpu();

    puts("\nCerrando CPU...");

    return OK;
}

void liberar_cpu() {
    close(conexion_memoria);
    close(conexion_dispatch);
    close(conexion_interrupt);
    close(dispatch_server);
    close(interrupt_server);
    log_destroy(logger);
    log_destroy(extra_logger);
    config_destroy(config);
}

void load_cpu_config(String path) {

    config = iniciar_config(path);

    if(config == NULL) {
        fprintf(stderr, "Config invalido!\n");
        exit(EXIT_FAILURE);
    }

    cpu_config.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
    cpu_config.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
    cpu_config.puerto_escucha_dispath = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    cpu_config.puerto_escucha_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    cpu_config.cantidad_entradas = config_get_int_value(config,"CANTIDAD_ENTRADAS_TLB");
    cpu_config.algoritmo_tlb = config_get_string_value(config,"ALGORITMO_TLB");
}
