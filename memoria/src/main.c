#include "main.h"
t_log* logger;





int main(int argc, char* argv[]) {
    logger = iniciar_logger();
	t_config* config = iniciar_config();

//obtenemos los valores de la config

	int puerto_escucha = config_get_int_value(config, "PUERTO_ESCUCHA");
    int tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
	int tam_pagina = config_get_int_value(config, "TAM_PAGINA");
	char* path_intrucciones = config_get_string_value(config, "PATH_INSTRUCCIONES");
	int retardo_respuesta = config_get_int_value(config, "RETARDO_RESPUESTA");

//iniciamos servidor

	int server_fd = iniciar_servidor(puerto_escucha, logger);
	log_info(logger, "Memoria esta lista para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd, logger);

	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida.");
			break;
		}
	}

    return EXIT_SUCCESS;
}

t_config* iniciar_config(void) {
	t_config* nuevo_config = config_create("memoria.config");
	if(nuevo_config == NULL)
		error_exit("Error, create config");
	return nuevo_config;
}


t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("memoria.log","MEMORIA",1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL)
		error_exit("Error, create a new log.");
	return nuevo_logger;
}


void error_exit(char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}


// conexiones - funciones para el inicio del servidor

void iterator(char* value) {
	log_info(logger,"%s", value);
}

int iniciar_servidor(int puerto_escucha, t_log* logger)
{
    int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	printf("%d", puerto_escucha);
	getaddrinfo(NULL, string_itoa(puerto_escucha), &hints, &servinfo);

	// Creamos el socket de escucha del servidor

	socket_servidor = socket(servinfo->ai_family,
                        servinfo->ai_socktype,
                        servinfo->ai_protocol);

	// Asociamos el socket a un puerto

	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes

	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	
}
	
t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente); 
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}
