#ifndef CONEXION_H_
#define CONEXION_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#define ERROR -1

typedef const char* String;

/**
* @fn    Crea una conexion con el servidor
* @brief El cliente crea una conexion con el servidor en la ip y puerto especificado
*/
int crear_conexion(String ip, String puerto);

/**
* @fn    Inicia una conexión en un puerto dado
* @brief Crea un socket y queda escuchando el puerto indicado
*/
int iniciar_servidor(String puerto);

#endif
//true, LOG_LEVEL_INFO