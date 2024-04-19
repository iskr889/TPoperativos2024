#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
//#include <commons/log.h>
//#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <utils/hello.h>
#include "cliente/utils.h"

enum INTERFAZ_CASE{
    STDOUT,
    STDIN,
    GENERIC,
    DIALFS,
    ERROR
} interfaz_code;


typedef struct{
    char* tipo_interfaz;
    int tiempo_u_trabajo;
    char *ip_kernel;
    char *puerto_kernel;
    char *ip_memoria;
    int puerto_memoria;
    char *path_base_dialfs;
    int block_size;
    int block_count;
}t_interfaz_config;

t_log* iniciar_logger(void);
t_log* iniciar_logger_interfaz(char *);
t_config* iniciar_config(void);
t_interfaz_config* load_interfaz_config(t_config* config);
void error_exit(char *);


#endif