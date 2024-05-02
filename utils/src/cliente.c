
#include "cliente.h"

int crear_conexion(String ip, String puerto) {
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(ip, puerto, &hints, &server_info)) {
		perror("Error getaddrinfo");
		freeaddrinfo(server_info);
		return EXIT_ERROR;
	}

	int fd_cliente = socket(server_info->ai_family,
							server_info->ai_socktype,
							server_info->ai_protocol);

	if(fd_cliente < 0) {
		perror("Error en socket()");
		freeaddrinfo(server_info);
		return EXIT_ERROR;
	}
    
    if(connect(fd_cliente, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        perror("Error en connect()");
		freeaddrinfo(server_info);
		return EXIT_ERROR;
    }

	freeaddrinfo(server_info);

	return fd_cliente;
}

int handshake_con_servidor(int socket_servidor) {
	int32_t handshake = 1;
	int32_t result;

	if(send(socket_servidor, &handshake, sizeof(int32_t), 0) < 0)
		return EXIT_ERROR;
	if(recv(socket_servidor, &result, sizeof(int32_t), MSG_WAITALL) < 0)
		return EXIT_ERROR;

	return result; // Retorna 0 si el handshake es correcto
}

int conectarse_a_modulo(String modulo, String ip, String puerto, t_log* logger) {

	int fd_modulo = crear_conexion(ip, puerto);

    if (fd_modulo < 0) {
        log_error(logger, "NO SE PUDO CONECTAR CON EL MODULO %s", modulo);
        exit(EXIT_FAILURE);
    }

    log_info(logger, "CONECTADO A %s", modulo);

    return fd_modulo;
}
