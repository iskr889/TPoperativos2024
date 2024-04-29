#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>

//Para el servidor
#include "/home/utnso/tp-2024-1c-groupSO2024/utils/src/servidor.h"
#include "../../utils/src/utils.h"



typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

//config

typedef struct {
    String   puerto_escucha;
    String   tam_memoria;
    String   tam_pagina;
    String   path_intrucciones;
    String   retardo_respuesta;
} t_memoria_config;


t_memoria_config* load_memoria_config(t_config* config);
void memoria_config_destroy(t_memoria_config* memoria_config);

#endif
