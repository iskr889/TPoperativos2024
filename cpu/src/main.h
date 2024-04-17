#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <readline/readline.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
//#include <utils.h>
//#include "server.h"


t_log* iniciar_logger(void);
void terminar_programa(int, t_log*, t_config*);
int iniciar_servidor(t_log*, char*, char*);
void error_exit(char *);
void iterator(char*);
t_config* iniciar_config(void);
void get_config_info(t_log*, t_config*);
int crear_conexion(t_log*, char*, char*); 

typedef struct {
    char *ip_memoria;
    char *puerto_memoria;
    char *puerto_escucha_dispath;
    char *puerto_escucha_interrupt;
    char *cantidad_entradas;
    char *algoritmo_tlb;
} info_config_struct;

info_config_struct info_config;

//extern info_config_struct info_config;
#endif