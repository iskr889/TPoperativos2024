#include "main.h"

int main(int argc, char* argv[]) {
    t_log* logger = iniciar_logger();
	t_config* config = iniciar_config();
    char *ip_memoria = config_get_string_value(config,"IP_MEMORIA");
    int puerto_memoria = config_get_int_value(config,"PUERTO_MEMORIA");
    char *ip_cpu = config_get_string_value(config,"IP_CPU");
    int puerto_cpu_dispatch = config_get_int_value(config,"PUERTO_CPU_DISPATCH");
    int puerto_cpu_interrupt = config_get_int_value(config,"PUERTO_CPU_INTERRUPT");
    char *algoritmo_planificacion = config_get_string_value(config,"ALGORITMO_PLANIFICACION");
    int quantum = config_get_int_value(config,"QUANTUM");
    char** recursos = config_get_array_value(config,"RECURSOS");
    char** instancias_recursos = config_get_array_value(config,"INSTANCIAS_RECURSOS");
    int grado_multiprogramacion = config_get_int_value(config,"GRADO_MULTIPROGRAMACION");

    return 0;
}



t_config* iniciar_config(void) {
	t_config* nuevo_config = config_create("kernel.config");
	if(nuevo_config == NULL) error_exit("Error, create config");
	return nuevo_config;
}


t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("kernel.log","KERNEL",1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL) error_exit("Error, create a new log.");
	return nuevo_logger;
}


void error_exit(char *message) { perror(message); exit(EXIT_FAILURE); }