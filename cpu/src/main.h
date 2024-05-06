#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/utils.h"
#include "../../utils/src/servidor.h"
#include "../../utils/src/cliente.h"

/**
* @fn    Carga la configuración en la estructura cpu_config
* @brief Carga los valores correspondiente en la estructura cpu_config
*/
void load_cpu_config(t_config* config);

typedef struct {
    String ip_memoria;
    String puerto_memoria;
    String puerto_escucha_dispath;
    String puerto_escucha_interrupt;
    uint64_t cantidad_entradas;
    String algoritmo_tlb;
} t_cpu_config;

#endif