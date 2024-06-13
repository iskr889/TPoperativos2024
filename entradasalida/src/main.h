#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/cliente.h"
#include "../../utils/src/utils.h"
#include "../../utils/src/serializacion.h"

typedef struct {
    String tipo_interfaz;
    uint32_t tiempo_u_trabajo;
    String ip_kernel;
    String puerto_kernel;
    String ip_memoria;
    String puerto_memoria;
    String path_base_dialfs;
    uint32_t block_size;
    uint32_t block_count;
    uint32_t retraso_compactacion;
} t_interfaz_config;

/**
* @fn    Libera toda la interfaz
* @brief Cierra los fd y libera las estructuras interfaz_config, config y loggers
*/
void liberar_interfaz();

/**
* @fn    Carga la configuración en la estructura interfaz_config
* @brief Segun el tipo de interfaz carga a la estructura los datos correspondientes
*/
t_interfaz_config* load_io_config(String path);

/**
* @fn    Libera la estructura interfaz_config
* @brief Libera la estructura interfaz_config y todas sus punteros
*/
void io_config_destroy();

void enviar_nombre_interfaz(String nombre, int socket);

#endif
