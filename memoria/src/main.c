#include "main.h"
#include <pthread.h>
#include <unistd.h>

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);
	t_config* config = iniciar_config("memoria.config");
	t_memoria_config* memoria_config = load_memoria_config(config);

    // Iniciamos servidor escuchando por conexiones de CPU, KERNEL e INTERFACES
    int fd_memoria_server = modulo_escucha_conexiones_de("CPU, KERNEL e INTERFACES", memoria_config->puerto_escucha, logger);

    // Acepto clientes en un thread aparte asi no frena la ejecución del programa
    pthread_t thread_memoria;
    atender_conexiones_al_modulo(&thread_memoria, fd_memoria_server);

    sleep(10); // TODO: Borrar! Solo sirve para testear rapidamente la conexion entre modulos

    pthread_join(thread_memoria, NULL);

    close(fd_memoria_server);
	log_destroy(logger);
    config_destroy(config); // Libera la memoria de config
    free(memoria_config);
    
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