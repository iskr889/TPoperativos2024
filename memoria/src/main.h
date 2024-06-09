#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/servidor.h"
#include "../../utils/src/utils.h"
#include <commons/bitarray.h>

typedef struct {
    int marco;
    bool asignada;
} PageTable_t; // Tabla donde el indice de pagina representa el marco y si está asignado o no

typedef struct {
    uint16_t pid;
    t_list *paginas;
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

/**
* @fn    Inicializar memoria
* @brief Crea la memoria de espacio de usuario, crea el diccionario de procesos y el bitarray de marcos asignados
*/
void inicializar_memoria();

/**
* @fn    Crea un proceso
* @brief Agrega el proceso al diccionario, le asigna un pid y una lista de instrucciones 
*/
void crear_proceso(uint16_t pid, t_list *instrucciones);

/**
* @fn    Libera la memoria del proceso
* @brief Lo elimina del diccionario, libera las paginas asignadas y la lista de instrucciones
*/
void liberar_proceso(uint16_t pid);

/**
* @fn    Libera una pagina de un proceso
* @brief Limpia el bitarray del frame asignado y hace un free de la pagina
*/
void liberar_pagina(void *page);

/**
* @fn    Libera todas las paginas de un proceso
* @brief Itera sobre todas las paginas y llama a liberar_pagina
*/
void liberar_paginas(t_list* paginas);

/**
* @fn    Asgina paginas a un proceso
* @brief Agrega las cantidad de paginas solicitada a la lista de paginas del proceso dado
*/
bool asignar_paginas(Proceso_t* proceso, uint32_t cant_paginas);

/**
* @fn    Accede al frame de un proceso
* @brief Accede al frame correspondiente al numero de pagina del proceso dado
*/
int acceder_marco(Proceso_t* proceso, uint32_t numero_de_pagina);

/**
* @fn    Permite ajustar la cantidad de paginas de un proceso
* @brief Ajusta la cantidad de paginas de un proceso, liberando paginas al reducir y asignando paginas al expandir
*/
bool resize_proceso(Proceso_t* proceso, uint32_t new_cant_paginas);

/**
* @fn    Lee una dirección fisica de la memoria de Usuario
* @brief Lee una dirección de tamaño size y la copia en buffer
*/
bool leer_memoria(uint32_t direccion_fisica, void *buffer, size_t size);

/**
* @fn    Escribe una dirección fisica en la memoria de Usuario
* @brief Escribe una dirección de tamaño size dado por data
*/
bool escribir_memoria(uint32_t direccion_fisica, void *data, size_t size);

#endif
