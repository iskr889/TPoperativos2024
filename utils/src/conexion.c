#include "conexion.h"

int crear_conexion(String ip, String puerto) {
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(ip, puerto, &hints, &server_info)) {
		perror("Error getaddrinfo");
		freeaddrinfo(server_info);
		return ERROR;
	}

	int fd_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	if(fd_cliente < 0) {
		perror("Error en socket()");
		freeaddrinfo(server_info);
		return ERROR;
	}
    
    if(connect(fd_cliente, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        perror("Error en connect()");
		freeaddrinfo(server_info);
		return ERROR;
    }

	freeaddrinfo(server_info);

	return fd_cliente;
}

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
