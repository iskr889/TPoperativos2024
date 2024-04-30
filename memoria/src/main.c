#include "main.h"

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("memoria.log", "MEMORIA",1,LOG_LEVEL_INFO);
	t_config* config = iniciar_config("memoria.config");
	t_memoria_config* memoria_config = load_memoria_config(config);


    //iniciamos servidor

    int fd_memoria_server = iniciar_servidor(memoria_config->puerto_escucha);

    if(fd_memoria_server < 0)
        return EXIT_ERROR;
    
    log_info(logger, "[MEMORIA] SERVIDOR INICIADO");

    pthread_t thread_id;

    // Acepto clientes en un thread 
    if(pthread_create(&thread_id, NULL, thread_aceptar_clientes, &fd_memoria_server) != 0) {
        perror("No se pudo crear el hilo");
        return EXIT_ERROR;
    }

    close(fd_memoria_server);
	log_destroy(logger);
    config_destroy(config); // Libera la memoria de config

    
    return 0;
}

//conexiones
t_memoria_config* load_memoria_config(t_config* config) {
    
    t_memoria_config* memoria_config = malloc(sizeof(t_memoria_config));
    
    if(memoria_config == NULL) {
        perror("Fallo malloc");
        exit(EXIT_FAILURE);
    }

	memoria_config->puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    memoria_config->tam_memoria = config_get_string_value(config,"TAM_MEMORIA");

    memoria_config->tam_pagina = config_get_string_value(config,"TAM_PAGINA");

    memoria_config->path_intrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");

    memoria_config->retardo_respuesta = config_get_string_value(config,"RETARDO_RESPUESTA");


	//obtenemos los valores de la config

	//int puerto_escucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    //int tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
	//int tam_pagina = config_get_int_value(config, "TAM_PAGINA");
	//char* path_intrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
	//int retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");

    return memoria_config;
}