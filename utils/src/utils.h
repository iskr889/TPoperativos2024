#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <pthread.h>
#include <string.h>

#define ERROR -1
#define OK     0

typedef const char* String;

/**
* @fn    Imprime un mensaje y sale del programa
* @brief Imprime un mensaje de error y sale del programa
*/
void error_exit(String message);

/**
* @fn    Inicia una conexión en un puerto dado
* @brief Crea un socket y queda escuchando el puerto indicado
*/
int iniciar_servidor(int puerto);

/**
* @fn    El servidor acepta a los clientes
* @brief Forkea el programa cada vez que acepta un cliente y continua escuchando por mas conexiones [BLOQUEANTE]
*/
int aceptar_clientes(int socket_servidor);

/**
* @fn    Handshake del servidor al cliente
* @brief Inicia un Handshake entre el cliente y el servidor
*/
int handshake_con_cliente(int socket_cliente);

/**
* @fn    Nuevo hilo para aceptar cliente
* @brief Crea un hilo de ejecución nuevo para aceptar clientes y que el programa pueda continua su ejecución
*/
void *thread_aceptar_clientes(void *arg);

/**
* @fn    Inicia un logger
* @brief Crea un logger y devuelve un puntero a t_log para ser utilizado
*/
t_log* iniciar_logger(void);

/**
* @fn    Inicia un config
* @brief Crea un config y devuelve un puntero a t_config para ser utilizado
*/
t_config* iniciar_config(void);

/**
* @fn    Crea una conexion con el servidor
* @brief El cliente crea una conexion con el servidor en la ip y puerto especificado
*/
int crear_conexion(String ip, int puerto);

/**
* @fn    Handshake del cliente al servidor
* @brief Inicia un Handshake entre el servidor y el cliente
*/
int handshake_con_servidor(int socket_servidor);

#endif
