#ifndef CONSOLA_H
#define CONSOLA_H

#include "main.h"

#define BUFF_SIZE 256

/**
* @fn    Imprime un mensaje de bienvenida
* @brief La consola imprime un mensaje de bienvenida
*/
void mensaje_de_bienvenida();

void ejecutar_script(const String path);

void iniciar_proceso(const String path);

void finalizar_proceso(const String pid);

void iniciar_planificacion(const String _);

void detener_planificacion(const String _);

void multiprogramacion(const String valor);

void proceso_estado(const String _);

void imprimir_proceso(void* proceso);

void sem_post_interfaces(char *key, void *element);

/**
* @fn    Recibe un comando y lo maneja
* @brief Recibe un comando y chequea que sea correcto para luego ejecutarlo
*/
void manejar_comando(const String command);

/**
* @fn    Ejecuta la consola
* @brief Función pasada al pthread_create para ejecutar la consola
*/
void* thread_consola(void* arg);

/**
* @fn    Ejecuta la consola en un thread distinto
* @brief Crea un thread y ejecuta la consola de forma no bloqueante
*/
int consola_kernel();

void crear_proceso_en_memoria(uint16_t pid, String path);

void finalizar_proceso_en_memoria(uint16_t pid);

#endif