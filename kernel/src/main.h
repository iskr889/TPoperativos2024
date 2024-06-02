#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/utils.h"
#include "../../utils/src/servidor.h"
#include "../../utils/src/cliente.h"
#include "../../utils/src/pcb.h"

typedef struct {
    String   puerto_escucha;
    String   ip_memoria;
    String   puerto_memoria;
    String   ip_cpu;
    String   puerto_cpu_dispatch;
    String   puerto_cpu_interrupt;
    String   algoritmo_planificacion;
    uint16_t quantum;
    String*  recursos;
    String*  instancias_recursos;
    uint16_t grado_multiprogramacion;
} t_kernel_config;

/**
* @fn    Carga la configuración en la estructura kernel_config
* @brief Abre un archivo config en path y lo carga en la estructura kernel_config
*/
t_kernel_config* load_kernel_config(String path);

/**
* @fn    Libera la memoria de la estructura kernel_config
* @brief Hace un free de la memoria solicitada incluido las variables de recursos e instancias_recursos
*/
void kernel_config_destroy();

/**
* @fn    Libera todo el kernel
* @brief Libera la memoria y todos los archivos usados por el kernel
*/
void liberar_kernel();

#endif