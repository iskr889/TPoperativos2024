#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/utils.h"

typedef struct {
    uint16_t puerto_escucha;
    String   ip_memoria;
    uint16_t puerto_memoria;
    String   ip_cpu;
    uint16_t puerto_cpu_dispatch;
    uint16_t puerto_cpu_interrupt;
    String   algoritmo_planificacion;
    uint16_t quantum;
    char**   recursos;
    char**   instancias_recursos;
    uint16_t grado_multiprogramacion;
} t_kernel_config;

t_kernel_config* load_kernel_config(t_config* config);

#endif