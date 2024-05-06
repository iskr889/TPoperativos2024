#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/servidor.h"
#include "../../utils/src/utils.h"

typedef struct {
    String puerto_escucha;
    String tam_memoria;
    String tam_pagina;
    String path_intrucciones;
    String retardo_respuesta;
} t_memoria_config;

/**
* @fn    Carga la configuración en la estructura memoria_config
* @brief Con el archivo config abierto solicita memoria y lo carga en la estructura memoria_config
*/
t_memoria_config* load_memoria_config(t_config* config);

#endif
