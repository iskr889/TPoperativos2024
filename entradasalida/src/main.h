#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/cliente.h"
#include "../../utils/src/utils.h"

typedef struct{
    String tipo_interfaz;
    uint64_t tiempo_u_trabajo;
    String ip_kernel;
    String puerto_kernel;
    String ip_memoria;
    String puerto_memoria;
    String path_base_dialfs;
    uint64_t block_size;
    uint64_t block_count;
} t_interfaz_config;

/**
* @fn    Carga la configuración en la estructura interfaz_config
* @brief Con el archivo config abierto solicita memoria y lo carga en la estructura interfaz_config
*/
t_interfaz_config* load_interfaz_config(t_config* config);

#endif