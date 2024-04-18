#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <assert.h>
#include <string.h>

typedef const char* String; // Pasar a utils

void error_exit(char *message); // Pasar a utils

/**
* @fn    Inicia una conexión en un puerto dado
* @brief Crea un socket y queda escuchando el puerto indicado
*/
int iniciar_servidor(int puerto);

/**
* @fn    El servidor acepta a los clientes
* @brief El servidor forkea el programa cada vez que acepta un cliente y continua escuchando por mas conexiones [BLOQUEANTE]
*/
int aceptar_clientes(int socket_servidor);

/**
* @fn    Handshake con el cliente
* @brief Inicia un Handshake entre el cliente y el servidor
*/
int handshake_con_cliente(int socket_cliente);

void* recibir_buffer(int*, int);

void recibir_mensaje(int, t_log*);

t_list* recibir_paquete(int);

#endif /* SERVER_H_ */
