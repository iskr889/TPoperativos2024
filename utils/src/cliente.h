#ifndef CLIENTE_H_
#define CLIENTE_H_

#include <stdio.h> // perror()

#include <sys/socket.h> // connect()
#include <sys/types.h> // connect()
#include <netdb.h> // getaddrinfo()

#include <string.h> // memset()

#define EXIT_ERROR -1
#define EXIT_OK     0

typedef char* String;

/**
* @fn    Crea una conexion con el servidor
* @brief El cliente crea una conexion con el servidor en la ip y puerto especificado
*/
int crear_conexion(String ip, String puerto);

/**
* @fn    Handshake del cliente al servidor
* @brief Inicia un Handshake entre el servidor y el cliente
*/
int handshake_con_servidor(int socket_servidor);

#endif
