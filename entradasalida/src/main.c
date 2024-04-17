#include "main.h"
//HOLA


int main(int argc, char* argv[]) {

    char* interfaz_name = "HOLA";

    int conexion_kernel = 0;
    int conexion_memoria = 0;

    t_log* logger = iniciar_logger();

	t_config* config = iniciar_config();

    //cargo archivo configuracion

    char *tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    int tiempo_u_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    char *ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char *puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char *ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    int puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    char *path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    int block_size = config_get_int_value(config, "BLOCK_SIZE");
    int block_count = config_get_int_value(config, "BLOCK_COUNT");

    enum INTERFAZ_CASE code = tipo_interfaz;

    switch(code){
        case STDIN:
            conexion_kernel = crear_conexion(ip_kernel , puerto_kernel);
            conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

            //IO_STDIN_READ
            //TEXT_STDIN

        case STDOUT:
            conexion_kernel = crear_conexion(ip_kernel , puerto_kernel);
            conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

            //IO_STDOUT_WRITE
            //DIREC_READ
        case GENERIC:
            conexion_kernel = crear_conexion(ip_kernel , puerto_kernel);

            //IO_GEN_SLEEP
        case DIALFS:
            conexion_kernel = crear_conexion(ip_kernel , puerto_kernel);
            conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);

            //IO_FS_CREATE
            //IO_FE_DELETE
            //IO_FS_TRUNCATE
            //IO_FS_WRITE
            //IO_FS_READ
            //INFO_READ
            //INFO_WRITE
    }

    //Creo conexion con el puerto kernel
    conexion_kernel = crear_conexion(ip_kernel , puerto_kernel);
    if(conexion_kernel == -1){
        log_error(logger, "Error creando socket para %s %s", ip_kernel, puerto_kernel);
    }

    //Creo conexion con el puerto memoria
    conexion_memoria = crear_conexion(ip_memoria, puerto_memoria);
    if(conexion_memoria == -1){
        log_error(logger, "Error creando socket para %s %s", ip_memoria, puerto_memoria);
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


void error_exit(char *message) { perror(message); exit(EXIT_FAILURE); }
