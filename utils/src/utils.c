#include "utils.h"

void error_exit(String message)
{ 
    perror(message);
    exit(EXIT_FAILURE);
}

int iniciar_servidor(String puerto)
{
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if(getaddrinfo(NULL, puerto, &hints, &server_info))
	{
		perror("Error en getaddrinfo");
		freeaddrinfo(server_info);
		return ERROR;
	}

	// Creamos el socket de escucha del servidor
	int socket_servidor = socket(server_info->ai_family,
							 server_info->ai_socktype,
							 server_info->ai_protocol);

	if(socket_servidor < 0)
	{
		perror("Error en socket()");
		freeaddrinfo(server_info);
		return ERROR;
	}

	// Asociamos el socket a un puerto
	if(bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen))
	{
		perror("Error en bind()");
		freeaddrinfo(server_info);
		return ERROR;
	}

	// Escuchamos las conexiones entrantes
	if(listen(socket_servidor, 1))
	{
		perror("Error en listen()");
		freeaddrinfo(server_info);
		return ERROR;
	}

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

        if(pid < 0)
		{
            close(socket_servidor);
            close(socket_cliente);
            perror("Error en fork()");
			return ERROR;
        }

        if(pid == 0) // Si entra al if es proceso hijo
        {
            close(socket_servidor); // El hijo cierra el fd porque no lo necesita
            handshake_con_cliente(socket_cliente);
            close(socket_cliente);
            exit(OK);
        }

		printf("Cliente conectado [PID:%d]", pid);
        close(socket_cliente); // Cierro el fd del cliente para poder aceptar otra connecion
    }

    if(socket_cliente < 0)
	{
		perror("Error en accept()");
		return ERROR;
	}

    close(socket_servidor);
	return OK;
}

int handshake_con_cliente(int socket_cliente)
{
	size_t bytes;
	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	if(recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL) < 0)
	{
		close(socket_cliente);
		return ERROR;
	}

	if(handshake == 1)
		bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
	else
		bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);

	if(bytes < 0)
	{
		close(socket_cliente);
		return ERROR;
	}

	return OK;
}

void *thread_aceptar_clientes(void *arg) 
{
    int socket_servidor = *(int *)arg;
    aceptar_clientes(socket_servidor);
    return NULL;
}

t_log* iniciar_logger(char *path, char *name) {
	//"kernel.log"
    t_log* nuevo_logger = log_create(path, name, true, LOG_LEVEL_INFO);
	
    if(nuevo_logger == NULL)
        error_exit("Error al tratar de crear kernel.log");
	
    return nuevo_logger;
}

t_config* iniciar_config(void) {
	
    t_config* nuevo_config = config_create("kernel.config");
	
    if(nuevo_config == NULL)
        error_exit("Error al tratar de crear kernel.config");
	
    return nuevo_config;
}

int crear_conexion(String ip, String puerto)
{
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(ip, puerto, &hints, &server_info))
	{
		perror("Errostring_itoar en getaddrinfo");
		freeaddrinfo(server_info);
		return ERROR;
	}

	int fd_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	if(fd_cliente < 0)
	{
		perror("Error en socket()");
		freeaddrinfo(server_info);
		return ERROR;
	}
    
    if(connect(fd_cliente, server_info->ai_addr, server_info->ai_addrlen) < 0)
	{
        perror("Error en connect()");
		freeaddrinfo(server_info);
		return ERROR;
    }

	freeaddrinfo(server_info);

	return fd_cliente;
}

int handshake_con_servidor(int socket_servidor)
{
	int32_t handshake = 1;
	int32_t result;

	if(send(socket_servidor, &handshake, sizeof(int32_t), 0) < 0)
		return ERROR;
	if(recv(socket_servidor, &result, sizeof(int32_t), MSG_WAITALL))
		return ERROR;

	return result; // Retorna 0 si el handshake es correcto
}