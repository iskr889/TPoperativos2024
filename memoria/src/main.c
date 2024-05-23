#include "main.h"
#include "memory_interface_handler.h"

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);
    t_config* config = iniciar_config("memoria.config");
    t_memoria_config* memoria_config = load_memoria_config(config);

    log_info(logger, "Archivo de configuración cargado correctamente");

    // Iniciamos servidor escuchando por conexiones de CPU, KERNEL e INTERFACES
    int memoria_server = escuchar_conexiones_de("CPU, KERNEL e INTERFACES", memoria_config->puerto_escucha, logger);

    // La MEMORIA espera que la CPU se conecte
    int conexion_cpu = esperar_conexion_de(CPU_CON_MEMORIA, memoria_server);

    if(conexion_cpu < 0)
        log_error(logger, "MODULO CPU NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_info(logger, "MODULO CPU CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // La MEMORIA espera que el KERNEL se conecte
    int conexion_kernel = esperar_conexion_de(KERNEL_CON_MEMORIA, memoria_server);

    if(conexion_kernel < 0)
        log_error(logger, "MODULO KERNEL NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_info(logger, "MODULO KERNEL CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // Acepto interfaces en un thread aparte asi no frena la ejecución del programa
    manejador_de_interfaces(memoria_server);

    sleep(30);

    // Cierro todos lo archivos y libero los punteros usados
    close(memoria_server);
    log_destroy(logger);
    config_destroy(config);
    free(memoria_config);
    
    return OK;
}

t_memoria_config* load_memoria_config(t_config* config) {
    
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