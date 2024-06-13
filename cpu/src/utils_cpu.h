#ifndef UTILS_CPU_H
#define UTILS_CPU_H

#include "../../utils/src/utils.h"
#include "../../utils/src/pcb.h"
#include "../../utils/src/instruccion.h"
#include "../../utils/src/servidor.h"
#include "../../utils/src/cliente.h"


void liberarConfig(t_config*);
// void mostrarConfig();
void load_cpu_config(t_config* temp_config);
void load_logger();
typedef struct {
    String ip_memoria;
    String puerto_memoria;
    String puerto_escucha_dispath;
    String puerto_escucha_interrupt;
    uint64_t cantidad_entradas;
    String algoritmo_tlb;
} t_cpu_config;

extern t_cpu_config config;
extern t_log*  logger;
#endif