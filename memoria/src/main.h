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
* @fn    Libera toda la memoria
* @brief Cierra los fd y libera las estructuras memoria_config, config y loggers
*/
void liberar_memoria();

/**
* @fn    Carga la configuración en la estructura memoria_config
* @brief Abre un archivo config en path y lo carga en la estructura memoria_config
*/
t_memoria_config* load_memoria_config(String path);

#endif
