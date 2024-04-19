#include "main.h"

/*#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>*/
//#include "cliente/utils.c"


int main(int argc, char* argv[]) {

    char *interfaz_name = "Interfaz_STDOUT";

    int conexion_kernel = 0;
    int conexion_memoria = 0;

    t_log* logger = iniciar_logger_interfaz(interfaz_name);

	t_config* config = iniciar_config();

    t_interfaz_config* config_interfaz = load_interfaz_config(config);


    if (strcmp(config_interfaz->tipo_interfaz, "STDOUT") == 0) {
            conexion_kernel = crear_conexion(config_interfaz->ip_kernel , config_interfaz->puerto_kernel, logger, "KERNEL");
            log_info(logger, "Se conecto correctamente a KERNEL");

            conexion_memoria = crear_conexion(config_interfaz->ip_memoria, config_interfaz->puerto_memoria, logger, "MEMORIA");
            log_info(logger, "Se conecto correctamente a MEMORIA");

            //IO_STDOUT_WRITE
            //DIREC_READ
    } else if (strcmp(config_interfaz->tipo_interfaz, "STDIN") == 0) {
            conexion_kernel = crear_conexion(config_interfaz->ip_kernel , config_interfaz->puerto_kernel, logger, "KERNEL");
            log_info(logger, "Se conecto correctamente a KERNEL");
            

            conexion_memoria = crear_conexion(config_interfaz->ip_memoria, config_interfaz->puerto_memoria, logger, "MEMORIA");
            log_info(logger, "Se conecto correctamente a MEMORIA");

            //IO_STDIN_READ
            //TEXT_STDIN
    } else if (strcmp(config_interfaz->tipo_interfaz, "GENERIC") == 0) {
            conexion_kernel = crear_conexion(config_interfaz->ip_kernel , config_interfaz->puerto_kernel, logger, "KERNEL");
            log_info(logger, "Se conecto correctamente a KERNEL");

            //IO_GEN_SLEEP
    } else if (strcmp(config_interfaz->tipo_interfaz, "DIALFS") == 0) {
            conexion_kernel = crear_conexion(config_interfaz->ip_kernel, config_interfaz->puerto_kernel, logger, "KERNEL");
            log_info(logger, "Se conecto correctamente a KERNEL");

            conexion_memoria = crear_conexion(config_interfaz->ip_memoria, config_interfaz->puerto_memoria, logger, "MEMORIA");
            log_info(logger, "Se conecto correctamente a MEMORIA");

            //IO_FS_CREATE
            //IO_FE_DELETE
            //IO_FS_TRUNCATE
            //IO_FS_WRITE
            //IO_FS_READ
            //INFO_READ
            //INFO_WRITE
    }
     else {
        log_info(logger, "EL TIPO de interfaz es incorrecto");
    }

    return 0;
}

t_config* iniciar_config(void) {
	t_config* nuevo_config = config_create("entradasalida.config");
	if(nuevo_config == NULL) error_exit("Error, create config");
	return nuevo_config;
}


t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("entradasalida.log","IO",1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL) error_exit("Error, create a new log.");
	return nuevo_logger;
}

t_log* iniciar_logger_interfaz(char *interfaz_name) {
	t_log* nuevo_logger = log_create("entradasalida.log",interfaz_name,1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL) error_exit("Error, create a new log.");
	return nuevo_logger;
}



t_interfaz_config* load_interfaz_config(t_config* config){
    t_interfaz_config* interfaz_config = malloc(sizeof(t_interfaz_config));

    if(interfaz_config == NULL) {
        perror("Fallo malloc");
        exit(EXIT_FAILURE);
    }

    interfaz_config->tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    interfaz_config->tiempo_u_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfaz_config->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    interfaz_config->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    interfaz_config->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    interfaz_config->puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    interfaz_config->path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    interfaz_config->block_size = config_get_int_value(config, "BLOCK_SIZE");
    interfaz_config->block_count = config_get_int_value(config, "BLOCK_COUNT");

    
    return interfaz_config;
}

void error_exit(char *message) { perror(message); exit(EXIT_FAILURE); }
