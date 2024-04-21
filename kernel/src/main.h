#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/utils.h"
#include "../../utils/src/conexion.h"
#include "../../utils/src/server.h"
#include "../../utils/src/client.h"

typedef struct {
    String   puerto_escucha;
    String   ip_memoria;
    String   puerto_memoria;
    String   ip_cpu;
    String   puerto_cpu_dispatch;
    String   puerto_cpu_interrupt;
    String   algoritmo_planificacion;
    uint16_t quantum;
    char**   recursos;
    char**   instancias_recursos;
    uint16_t grado_multiprogramacion;
} t_kernel_config;

t_kernel_config* load_kernel_config(t_config* config);
void kernel_config_destroy(t_kernel_config* kernel_config);
#endif