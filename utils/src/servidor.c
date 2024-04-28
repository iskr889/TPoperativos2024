#include "servidor.h"
#include <stdio.h>

int iniciar_servidor(String puerto) {
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if(getaddrinfo(NULL, puerto, &hints, &server_info)) {
		perror("Error en getaddrinfo");
		freeaddrinfo(server_info);
		return ERROR;
	}

	// Creamos el socket de escucha del servidor
	int socket_servidor = socket(server_info->ai_family,
								 server_info->ai_socktype,
								 server_info->ai_protocol);

	if(socket_servidor < 0) {
		perror("Error en socket()");
		freeaddrinfo(server_info);
		return ERROR;
	}

	// Asociamos el socket a un puerto
	if(bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen)) {
		perror("Error en bind()");
		freeaddrinfo(server_info);
		return ERROR;
	}

	// Escuchamos las conexiones entrantes
	if(listen(socket_servidor, 1)) {
		perror("Error en listen()");
		freeaddrinfo(server_info);
		return ERROR;
	}

	freeaddrinfo(server_info);

	return socket_servidor;
}

int aceptar_clientes(int socket_servidor) {
	int socket_cliente, pid;

	// Aceptamos un nuevo cliente
	while((socket_cliente = accept(socket_servidor, NULL, NULL)) > 0) {     
		pid = fork();

        if(pid < 0) {
            close(socket_servidor);
            close(socket_cliente);
            perror("Error en fork()");
			return ERROR;
        }

        if(pid == 0) { // Si entra al if es proceso hijo
            close(socket_servidor); // El hijo cierra el fd porque no lo necesita
            handshake_con_cliente(socket_cliente);
            close(socket_cliente);
            exit(OK);
        }

		printf("Cliente conectado [PID:%d]", pid);
        close(socket_cliente); // Cierro el fd del cliente para poder aceptar otra connecion
    }

    if(socket_cliente < 0) {
		perror("Error en accept()");
		return ERROR;
	}

    close(socket_servidor);
	return OK;
}

int handshake_con_cliente(int socket_cliente) {
	size_t bytes;
	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	if(recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL) < 0) {
		perror("Error en handshake");
		close(socket_cliente);
		return ERROR;
	}

	if(handshake == 1)
		bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
	else
		bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);

	if(bytes < 0) {
		perror("Error en handshake");
		close(socket_cliente);
		return ERROR;
	}

	return OK;
}

void *thread_aceptar_clientes(void *arg) {
    int socket_servidor = *(int *)arg;
    aceptar_clientes(socket_servidor);
    return NULL;
}



