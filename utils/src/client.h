#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#include <commons/collections/list.h>
#include <commons/string.h>

#define ERROR -1
#define OK     0

typedef const char* String;


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

#endif
