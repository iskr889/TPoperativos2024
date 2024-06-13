#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/utils.h"
#include "../../utils/src/servidor.h"
#include "../../utils/src/cliente.h"
#include "../../utils/src/pcb.h"
#include "../../utils/src/instruccion.h"
#include "../../utils/src/serializacion.h"

/**
* @fn    Libera todo la cpu
* @brief Cierra los fd y libera las estructuras config y loggers
*/
void liberar_cpu();

/**
* @fn    Carga la configuración en la estructura cpu_config
* @brief Abre un archivo config en path y lo carga en la estructura cpu_config
*/
void load_cpu_config(String path);

typedef struct {
    String ip_memoria;
    String puerto_memoria;
    String puerto_escucha_dispath;
    String puerto_escucha_interrupt;
    uint64_t cantidad_entradas;
    String algoritmo_tlb;
} t_cpu_config;

#endif