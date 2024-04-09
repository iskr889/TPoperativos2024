#include "main.h"


int main(int argc, char* argv[]) {
    t_log* logger = iniciar_logger();
	t_config* config = iniciar_config();

    char *tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    int tiempo_u_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    char *ip_kernel = config_get_string_value(config, "IP_KERNEL");
    char *puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    char *ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    int puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    char *path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    int block_size = config_get_int_value(config, "BLOCK_SIZE");
    int block_count = config_get_int_value(config, "BLOCK_COUNT");

    return 0;
}

t_config* iniciar_config(void) {
	t_config* nuevo_config = config_create("entradasalida.config");
	if(nuevo_config == NULL) error_exit("Error, create config");
	return nuevo_config;
}


t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("entradasalida.log","KERNEL",1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL) error_exit("Error, create a new log.");
	return nuevo_logger;
}


void error_exit(char *message) { perror(message); exit(EXIT_FAILURE); }