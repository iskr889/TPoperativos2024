#include "server.h"

// t_log* logger;

// int iniciar_servidor(char *puerto, char *type) {
// 	printf("puerto: %s\n", puerto);
// 	logger = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
// 	int conexion;
// 	struct addrinfo hints, *server_info;
// 	int err;
//     memset(&hints, 0, sizeof(hints));
// 	hints.ai_family = AF_INET;
// 	hints.ai_socktype = SOCK_STREAM;
// 	hints.ai_flags = AI_PASSIVE;

// 	err = getaddrinfo(NULL, puerto, &hints, &server_info);
// 	if(err){
//         perror("Error al intentar crear un socket de tipo \n"); 
//         exit(EXIT_FAILURE);
//     }

//     conexion = socket(server_info->ai_family,
//                          server_info->ai_socktype,
//                          server_info->ai_protocol);


// 	printf("errr %d\n", conexion);
//     if (conexion) {
//         perror("Error al crear el socket\n");
//         exit(EXIT_FAILURE);
//     }

// 	err = bind(conexion, server_info->ai_addr, server_info->ai_addrlen);
// 	if (err) {
//         perror("Error al vincular el socket\n");
//         exit(EXIT_FAILURE);
//     }

// 	err = listen(conexion, SOMAXCONN);
// 	if (err) {
//         perror("Error al poner en escucha el socket\n");
//         exit(EXIT_FAILURE);
//     }
	
	
// 	freeaddrinfo(server_info);
// 	log_trace(logger, "Socket de tipo generado");
// 	return conexion;
// }



// int esperar_user(int socket_servidor, char *type) {
// 	int socket_cliente = accept(socket_servidor, NULL, NULL);
// 	log_info(logger, "Conexion establecida en %s\n", type);
// 	return socket_cliente;
// }

// int recibir_solicitud(int socket_cliente) {
// 	int cod_op;
// 	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
// 		return cod_op;
// 	else {
// 		close(socket_cliente);
// 		return -1;
// 	}
// }
