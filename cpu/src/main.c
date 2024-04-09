#include "main.h"

int main(int argc, char* argv[]) {
    decir_hola("CPU");

    t_log* logger = iniciar_logger();
	t_config* config = iniciar_config();

    char *ip_memoria = config_get_string_value(config,"IP_MEMORIA");
    int puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
    int puerto_escucha_dispath = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    int puerto_escucha_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    int cantidad_entradas = config_get_string_value(config,"CANTIDAD_ENTRADAS_TLB");
    char *algoritmo_tlb = config_get_string_value(config,"ALGORITMO_TLB");

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