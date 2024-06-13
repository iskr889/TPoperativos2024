#include "utils_cpu.h"

void load_cpu_config(t_config* temp_config) {
    config.ip_memoria = config_get_string_value(temp_config, "IP_MEMORIA");
    config.puerto_memoria = config_get_string_value(temp_config, "PUERTO_MEMORIA");
    config.puerto_escucha_dispath = config_get_string_value(temp_config, "PUERTO_ESCUCHA_DISPATCH");
    config.puerto_escucha_interrupt = config_get_string_value(temp_config, "PUERTO_ESCUCHA_INTERRUPT");
    config.cantidad_entradas = config_get_int_value(temp_config, "CANTIDAD_ENTRADAS_TLB");
    config.algoritmo_tlb = config_get_string_value(temp_config, "ALGORITMO_TLB");
}
void liberarConfig(t_config* temp_config) {
    config_destroy(temp_config);
}


void load_logger() {
    logger = iniciar_logger("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
}