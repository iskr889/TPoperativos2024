#ifndef MANEJARINSTRUCCIONES_H_
#define MANEJARINSTRUCCIONES_H_

#include "main.h"
#include "../../utils/src/serializacion.h"

#define CARPETA_PSEUDOCODIGO "pseudocodigo/" // Carpeta donde estan los archivos de pseudocodigo
#define BUFF_SIZE 256 // Tamaño del buffer que lee una linea de pseudocodigo a la vez
#define CANT_PAGINAS_DEFAULT 1 // Cantidad de paginas por default asignadas a un proceso recien creado

/**
* @fn    Hilo que maneja las instrucciones con el kernel
* @brief En un hilo distinto recibe los paquetes del kernel y según el codigo de operación ejecuta una instrucción
*/
void *thread_instrucciones_kernel(void *);

/**
* @fn    Maneja las instrucciones con el kernel
* @brief Crea un hilo que maneja las instrucciones con el kernel
*/
void manejar_instrucciones_kernel();

/**
* @fn    Ejecuta la instrucción MEMORY_PROCESS_CREATE recibida del kernel
* @brief Lee las instrucciones del pseudocodigo y las carga en la memoria junto con el pid recibido
*/
void instruccion_process_create(payload_t* payload);

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
