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

void* recibir_buffer(int*, int);

int iniciar_servidor(int, t_log*, char*);
int esperar_cliente(int, t_log*, char*);
t_list* recibir_paquete(int);
void recibir_mensaje(int, t_log*);
int recibir_operacion(int);

#endif /* SERVER_H_ */
