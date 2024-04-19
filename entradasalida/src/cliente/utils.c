#include "utils.h"


int crear_conexion(char *ip,  int puerto, t_log* logger, char* nombre_servidor)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, string_itoa(puerto), &hints, &server_info);

	// Ahora vamos a crear el socket.
	//int socket_cliente = 0;
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	
    
    int conexion = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

    if(conexion == -1 ) {
        log_error(logger, "Error al conectarse a %s", nombre_servidor);
        if (errno == ECONNREFUSED) {
            log_info(logger, "El servidor %s rechazó la conexión\n", nombre_servidor);
        } else if (errno == ETIMEDOUT) {
            log_info(logger, "Se agotó el tiempo de espera al intentar conectarse a %s\n", nombre_servidor);
        }
        exit(EXIT_FAILURE);
    }

	log_info(logger, "Se conecto correctamente a %s", nombre_servidor);

    
	freeaddrinfo(server_info);

	return socket_cliente;
}


