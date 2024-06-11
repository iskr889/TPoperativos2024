#ifndef MANEJARINSTRUCCIONES_H_
#define MANEJARINSTRUCCIONES_H_

#include "main.h"
#include "../../utils/src/serializacion.h"

#define CARPETA_PSEUDOCODIGO "pseudocodigo/" // Carpeta donde estan los archivos de pseudocodigo
#define BUFF_SIZE 256 // Tamaño del buffer que lee una linea de pseudocodigo a la vez

/**
* @fn    Maneja las instrucciones con el kernel
* @brief Crea un hilo que maneja las instrucciones con el kernel
*/
void manejar_instrucciones_kernel();

/**
* @fn    Hilo que maneja las instrucciones con el kernel
* @brief En un hilo distinto recibe los paquetes del kernel y según el codigo de operación ejecuta una instrucción
*/
void *thread_instrucciones_kernel(void *);

/**
* @fn    Maneja las instrucciones con la CPU
* @brief Crea un hilo que maneja las instrucciones con la CPU
*/
void manejar_instrucciones_cpu();

/**
* @fn    Hilo que maneja las instrucciones con la CPU
* @brief En un hilo distinto recibe los paquetes de la CPU y según el codigo de operación ejecuta una instrucción
*/
void *thread_instrucciones_cpu(void *);

/**
* @fn    Ejecuta la instrucción MEMORY_PROCESS_CREATE recibida del kernel
* @brief Lee las instrucciones del pseudocodigo y las carga en la memoria junto con el pid recibido
*/
void instruccion_process_create(payload_t* payload);

/**
* @fn    Ejecuta la instrucción MEMORY_PROCESS_TERM recibida del kernel
* @brief Libera el proceso con pid recibido del Kernel
*/
void instruccion_process_terminate(payload_t* payload);

/**
* @fn    Ejecuta la instrucción MEMORY_PAGE_TABLE_ACCESS recibida de la cpu
* @brief Accede a la tabla de paginas consultada
*/
void instruccion_pageTable_access(payload_t* payload);

/**
* @fn    Ejecuta la instrucción MEMORY_PROCESS_RESIZE recibida de la cpu
* @brief Ajusta la cantidad de paginas asignadas a un proceso
*/
void instruccion_process_resize(payload_t* payload);

/**
* @fn    Ejecuta la instrucción MEMORY_USER_SPACE_ACCESS recibida de la cpu o interfaces
* @brief Accede al espacio de usuario
*/
void instruccion_userspace_access(payload_t* payload, int fd_conexion);

/**
* @fn    Lee un archivo de pseudocodigo
* @brief Devuelve las instrucciones del archivo de pseudocodigo en una lista donde el numero de instrucción comenzando de 0 corresponde al indice de la lista
*/
t_list *leer_pseudocodigo(String filename);

/**
* @fn    Imprime las instrucciones
* @brief Imprime todas las instrucciones de la lista de instrucciones recibida
*/
void imprimir_instrucciones(t_list* instrucciones);

/**
* @fn    Imprime una instruccion
* @brief Imprime la instruccion recibida
*/
void imprimir_instruccion(String instruccion);

/**
* @fn    Imprime la instrucción numero x
* @brief Imprime la instruccion numero x recibida como parametro
*/
void imprimir_instruccion_numero(t_list* instrucciones, uint32_t numero_de_linea);

#endif
