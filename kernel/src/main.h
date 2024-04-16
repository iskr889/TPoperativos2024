#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <string.h>

#include "server/server.h"
#include "cliente/cliente.h"

typedef char* String;

typedef struct {
    uint16_t puerto_escucha;
    String ip_memoria;
    uint16_t puerto_memoria;
    String ip_cpu;
    uint16_t puerto_cpu_dispatch;
    uint16_t puerto_cpu_interrupt;
    String algoritmo_planificacion;
    uint16_t quantum;
    String *recursos;
    String *instancias_recursos;
    uint16_t grado_multiprogramacion;
} t_kernel_config;

t_log* iniciar_logger(void);

t_config* iniciar_config(void);

t_kernel_config* load_kernel_config(t_config* config);

void error_exit(char *);

#endif