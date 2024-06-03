#include "main.h"
#include "memory_interface_handler.h"

t_config* config;
t_memoria_config* memoria_config;
t_log* logger;
t_log* extra_logger;

int conexion_cpu, conexion_kernel, memoria_server;

int main(int argc, char* argv[]) {

    memoria_config = load_memoria_config("memoria.config");

    logger = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);

    extra_logger = iniciar_logger("memoria_debug.log", "MEMORIA", 1, LOG_LEVEL_DEBUG);

    // Iniciamos servidor escuchando por conexiones de CPU, KERNEL e INTERFACES
    memoria_server = escuchar_conexiones_de("CPU, KERNEL e INTERFACES", memoria_config->puerto_escucha, extra_logger);

    // La MEMORIA espera que la CPU se conecte
    conexion_cpu = esperar_conexion_de(CPU_CON_MEMORIA, memoria_server);

    if(conexion_cpu < 0)
        log_error(extra_logger, "MODULO CPU NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_debug(extra_logger, "MODULO CPU CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // La MEMORIA espera que el KERNEL se conecte
    conexion_kernel = esperar_conexion_de(KERNEL_CON_MEMORIA, memoria_server);

    if(conexion_kernel < 0)
        log_error(extra_logger, "MODULO KERNEL NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_debug(extra_logger, "MODULO KERNEL CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // Acepto interfaces en un thread aparte asi no frena la ejecución del programa
    manejador_de_interfaces(memoria_server);

    sleep(30);

    liberar_memoria();

    puts("\nCerrando Memoria...");
    
    return OK;
}

void liberar_memoria() {
    close(memoria_server);
    log_destroy(logger);
    log_destroy(extra_logger);
    config_destroy(config);
    free(memoria_config);
}

t_memoria_config* load_memoria_config(String path) {

    config = iniciar_config(path);

    if(config == NULL) {
        fprintf(stderr, "Config invalido!\n");
        exit(EXIT_FAILURE);
    }
    
    t_memoria_config* memoria_config = malloc(sizeof(t_memoria_config));
    
    if(memoria_config == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }

    memoria_config->puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    memoria_config->tam_memoria = config_get_string_value(config,"TAM_MEMORIA");
    memoria_config->tam_pagina = config_get_string_value(config,"TAM_PAGINA");
    memoria_config->path_intrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
    memoria_config->retardo_respuesta = config_get_string_value(config,"RETARDO_RESPUESTA");

    return memoria_config;
}
