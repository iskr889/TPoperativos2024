#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/servidor.h"
#include "../../utils/src/utils.h"
#include <commons/bitarray.h>

#define PAGE_SIZE 128 // Tamaño de pagina
#define CANT_PAGINAS 128 // Cantidad de paginas
#define MEMORY_SIZE (PAGE_SIZE * CANT_PAGINAS)  // Tamaño total de memoria

typedef struct {
    int marco;
    bool asignada;
} Page_table_t; // Tabla donde el indice de pagina representa el marco y si está asignado o no

typedef struct {
    int pid;
    int cant_paginas;
    Page_table_t* pagina;
} Proceso_t;

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

void inicializar_memoria();

Proceso_t* crear_proceso(int pid, int cant_paginas);

int asignar_pagina(Proceso_t* proceso);

void liberar_paginas_proceso(Proceso_t* proceso);

int acceder_marco(Proceso_t* proceso, int numero_de_pagina);

int leer_memoria(int physical_address, void* buffer, size_t size);

int escribir_memoria(int physical_address, void* data, size_t size);

int resize_proceso(Proceso_t* proceso, int new_cant_paginas);

#endif
