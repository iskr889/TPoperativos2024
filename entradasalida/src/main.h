#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/cliente.h"
#include "../../utils/src/utils.h"

typedef struct {
    int tamanio_instruccion;
    char *instruccion;
    int u_trabajo;
} t_instruccion_generica;

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
} t_interfaz_config;

/**
* @fn    Libera todas las estructuras
* @brief Libera las estructuras interfaz_config, config y logger
*/
void liberar_todo();

/**
* @fn    Carga la configuración en la estructura interfaz_config
* @brief Segun el tipo de interfaz carga a la estructura los datos correspondientes
*/
t_interfaz_config* load_io_config();

/**
* @fn    Libera la estructura interfaz_config
* @brief Libera la estructura interfaz_config y todas sus punteros
*/
void io_config_destroy();

/*** INSTRUCCIONES **/
void io_gen_sleep(int unidades_trabajo);

/*** FUNCIONES AUXILIARES ***/
t_instruccion_generica *recibir_instruccion(int fd);
int enviar_nombre(String nombre, int socket);

/*** INTERFACES **/
void interfaz_generica(String nombre);

void interfaz_stdin(String nombre);

void interfaz_stdout(String nombre);

void interfaz_dialFS(String nombre);

#endif
