#ifndef SERVER_H_
#define SERVER_H_

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

#define ERROR -1
#define OK     0

typedef const char* String;

/**
* @fn    Handshake del cliente al servidor
* @brief Inicia un Handshake entre el servidor y el cliente
*/
int handshake_con_servidor(int socket_servidor);

#endif
