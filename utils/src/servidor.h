#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <stdio.h> // perror()
#include <stdlib.h> // exit()

#include <sys/socket.h> // socket(), accept()
#include <sys/types.h> // socket(), accept()
#include <netdb.h> // getaddrinfo()

#include <string.h> // memset()
#include <unistd.h> // fork(), close()
#include <pthread.h> // pthread_create()

#define ERROR -1
#define OK     0

typedef char* String;

/**
* @fn    Inicia una conexión en un puerto dado
* @brief Crea un socket y queda escuchando el puerto indicado
*/
int iniciar_servidor(String puerto);

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
* @brief Crea un hilo de ejecución nuevo para aceptar clientes y que el servidor pueda continuar su ejecución sin ser bloqueado
*/
void *thread_aceptar_clientes(void *arg);

#endif
