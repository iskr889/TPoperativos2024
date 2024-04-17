#include "utils.h"

// info_config_struct info_config;

// void get_config_info(void) {
//     t_config* config = iniciar_config();
//     info_config.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
//     info_config.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
//     info_config.puerto_escucha_dispath = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
//     info_config.puerto_escucha_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
//     info_config.cantidad_entradas = config_get_string_value(config,"CANTIDAD_ENTRADAS_TLB");
//     info_config.algoritmo_tlb = config_get_string_value(config,"ALGORITMO_TLB");
//     config_destroy(config);
// }


// t_config* iniciar_config(void) {
// 	t_config* nuevo_config = config_create("cpu.config");
// 	if(nuevo_config == NULL) error_exit("Error, create config");
// 	return nuevo_config;
// }