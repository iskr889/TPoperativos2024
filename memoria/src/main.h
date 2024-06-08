#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/servidor.h"
#include "../../utils/src/utils.h"
#include <commons/bitarray.h>

typedef struct {
    int marco;
    bool asignada;
} Page_table_t; // Tabla donde el indice de pagina representa el marco y si está asignado o no

typedef struct {
    uint16_t pid;
    uint32_t cant_paginas;
    Page_table_t* pagina;
    t_list *instrucciones;
} Proceso_t;

typedef struct {
    String puerto_escucha;
    uint32_t tam_memoria;
    uint32_t tam_pagina;
    String path_intrucciones;
    uint32_t retardo_respuesta;
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

void crear_proceso(uint16_t pid, uint32_t cant_paginas, t_list *instrucciones);

void finalizar_proceso(uint16_t pid);

int asignar_pagina(Proceso_t* proceso);

void liberar_paginas_proceso(Proceso_t* proceso);

int acceder_marco(Proceso_t* proceso, uint32_t numero_de_pagina);

int leer_memoria(uint32_t direccion_fisica, void *buffer, size_t size);

int escribir_memoria(uint32_t direccion_fisica, void *data, size_t size);

int resize_proceso(Proceso_t* proceso, uint32_t new_cant_paginas);

#endif
