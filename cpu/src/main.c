#include "main.h"


int main(int argc, char* argv[]) {
	t_log* logger = iniciar_logger();
	t_config* config = iniciar_config();
	get_config_info(logger, config);
	int dispatch_server = iniciar_servidor(logger, info_config.puerto_escucha_dispath, "Dispath");
	int interrupt_server = iniciar_servidor(logger, info_config.puerto_escucha_interrupt, "Interrupt");
	log_info(logger, "Servers Loaded");
	
	crear_conexion(logger, info_config.ip_memoria, info_config.puerto_memoria);
	terminar_programa(dispatch_server, logger, config);

    return 0;
}


t_log* iniciar_logger(void) {
	t_log* nuevo_logger = log_create("cpu.log","CPU",1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL) error_exit("Error, create a new log.");
	return nuevo_logger;
}


void error_exit(char* message) { perror(message); exit(EXIT_FAILURE); }


void terminar_programa(int conexion, t_log* logger, t_config* config) {
	log_destroy(logger);
	close(conexion);
	config_destroy(config);
}

int iniciar_servidor(t_log* logger, char* puerto, char* type) {	
	int conexion;
	struct addrinfo hints,* server_info;
    memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	int err = getaddrinfo(NULL, puerto, &hints, &server_info);
	if(err){
        printf("Error al intentar crear un socket de tipo"); 
        exit(EXIT_FAILURE);
    }

    conexion = socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);


    if (conexion == -1) {
        perror("Error al crear el socket\n");
        exit(EXIT_FAILURE);
    }

	err = bind(conexion, server_info->ai_addr, server_info->ai_addrlen);
	if (err) {
        perror("Error al vincular el socket\n");
        exit(EXIT_FAILURE);
    }

	err = listen(conexion, SOMAXCONN);
	if (err == -1) {
        perror("Error al poner en escucha el socket\n");
        exit(EXIT_FAILURE);
    }
	
	freeaddrinfo(server_info);
	log_info(logger, "Socket de tipo generado %s", type);
	return conexion;
}



int esperar_user(t_log* logger, int socket_servidor, char* type) {
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Conexion establecida en %s\n", type);
	return socket_cliente;
}

int recibir_solicitud(int socket_cliente) {
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else {
		close(socket_cliente);
		return -1;
	}
}

void get_config_info(t_log* logger, t_config* config) {
    info_config.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
    info_config.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
    info_config.puerto_escucha_dispath = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    info_config.puerto_escucha_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    info_config.cantidad_entradas = config_get_string_value(config,"CANTIDAD_ENTRADAS_TLB");
	info_config.algoritmo_tlb = config_get_string_value(config,"ALGORITMO_TLB");
	log_info(logger, "Configuracion cargada correctamente");
}

t_config* iniciar_config(void) {
	t_config* nuevo_config = config_create("cpu.config");
	if(nuevo_config == NULL) error_exit("Error, create config");
	return nuevo_config;
}




int crear_conexion(t_log* logger, char* ip, char* puerto) {
	struct addrinfo hints,* server_info;
	int err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family,
                         server_info->ai_socktype,
                         server_info->ai_protocol);

	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	freeaddrinfo(server_info);

	log_info(logger, "conexion establecida con el servidor: ip: ",ip);
	return socket_cliente;
}