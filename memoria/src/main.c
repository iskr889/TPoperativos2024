#include "main.h"

int main(int argc, char* argv[]) {
    t_log* logger = iniciar_logger();
	t_config* config = iniciar_config();

	int puerto_escucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    int tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
	int tam_pagina = config_get_int_value(config, "TAM_PAGINA");
	char* path_intrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
	int retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");

    return 0;
}



t_config* iniciar_config(void) {
	t_config* nuevo_config = config_create("memoria.config");
	if(nuevo_config == NULL)
		error_exit("Error, create config");
	return nuevo_config;
}


t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("memoria.log","MEMORIA",1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL)
		error_exit("Error, create a new log.");
	return nuevo_logger;
}


void error_exit(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}