#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
//#include <commons/log.h>
//#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>

#include "../../utils/src/cliente.h"
#include "../../utils/src/utils.h"

typedef struct{
    char* tipo_interfaz;
    int tiempo_u_trabajo;
    char *ip_kernel;
    char *puerto_kernel;
    char *ip_memoria;
    char *puerto_memoria;
    char *path_base_dialfs;
    int block_size;
    int block_count;
}t_interfaz_config;

t_log* iniciar_logger_interfaz(char *);;
t_interfaz_config* load_interfaz_config(t_config* config);

#endif