#include "utils.h"

t_log* iniciar_logger(char *path, char *name, bool is_active_console, t_log_level level) {
    t_log* nuevo_logger = log_create(path, name, is_active_console, level);
    if(nuevo_logger == NULL){
		fprintf(stderr, "[ERROR] iniciar_logger [%s]\n", path);
		exit(EXIT_FAILURE);
	}
	
    return nuevo_logger;
}

t_config* iniciar_config(char *path) {
    t_config* nuevo_config = config_create(path);
	if(nuevo_config == NULL){
		fprintf(stderr, "[ERROR] iniciar_config [%s]\n", path);
		exit(EXIT_FAILURE);
	}
    return nuevo_config;
}