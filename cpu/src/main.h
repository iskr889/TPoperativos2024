#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/utils.h"
#include "../../utils/src/servidor.h"
#include "../../utils/src/cliente.h"

void terminar_programa(int, int, t_log*, t_config*);
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

#endif