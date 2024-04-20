#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <utils/hello.h>

//Para el servidor
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>


typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

//config
t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void error_exit(char *);

void crear_proceso(void);
void terminar_proceso(void);
void acceso_tabla_paginas(void);
void ajustar_lenght_proceso(void);
void acceso_espacio_usuario(void);

//servidor memoria
void inicio_server_memoria(void);
void* recibir_buffer(int*, int);
int iniciar_servidor(int puerto_escucha, t_log* logger);
int esperar_cliente(int socket_servidor, t_log* logger);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void iterator(char* value);

#endif
