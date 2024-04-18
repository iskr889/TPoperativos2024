#include "server.h"

int iniciar_servidor(int puerto)
{
	int socket_servidor, error;

	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if(getaddrinfo(NULL, string_itoa(puerto), &hints, &server_info))
		error_exit("Error en getaddrinfo");

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(server_info->ai_family,
							 server_info->ai_socktype,
							 server_info->ai_protocol);

	if(socket_servidor < 0)
		error_exit("Error en socket()");

	// Asociamos el socket a un puerto
	if(bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen))
		error_exit("Error en bind()");

	// Escuchamos las conexiones entrantes
	if(listen(socket_servidor, 1))
		error_exit("Error en listen()");

	freeaddrinfo(server_info);

	return socket_servidor;
}

int aceptar_clientes(int socket_servidor)
{
	int socket_cliente, pid;
	// Aceptamos un nuevo cliente
	while((socket_cliente = accept(socket_servidor, NULL, NULL)) > 0)
    {     
		pid = fork();

        if(pid == -1)
		{
            close(socket_servidor);
			
            close(socket_cliente);

            error_exit("Error en fork()");
        }

        if(pid == 0) // Si entra al if es proceso hijo
        {
            close(socket_servidor); // El hijo cierra el fd porque no lo necesita

            handshake_con_cliente(socket_cliente);

            close(socket_cliente);

            return EXIT_SUCCESS;
        }

		printf("Cliente conectado [PID:%d]", pid);
        
        close(socket_cliente); // Cierro el fd del cliente para poder aceptar otra connecion
    }

    if(socket_cliente < 0)
		error_exit("Error en accept()");

    close(socket_servidor);

	return EXIT_SUCCESS;
}

int handshake_con_cliente(int socket_cliente)
{
	size_t bytes;

	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);

	if(bytes < 0) {
		close(socket_cliente);
		return -1;
	}

	if(handshake == 1)
		bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
	else
		bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);

	if(bytes < 0) {
		close(socket_cliente);
		return -1;
	}

	return EXIT_SUCCESS;
}
/*
void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
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
*/
void error_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}