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
#include "../../utils/src/utils.h"


void terminar_programa(int, t_log*, t_config*);
void iterator(char*);
void get_config_info(t_log*, t_config*);




typedef struct {
    char *ip_memoria;
    char *puerto_memoria;
    char *puerto_escucha_dispath;
    char *puerto_escucha_interrupt;
    char *cantidad_entradas;
    char *algoritmo_tlb;
} info_config_struct;

info_config_struct info_config;
#endif