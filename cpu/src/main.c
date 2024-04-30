#include "main.h"

info_config_struct info_config;

int main(int argc, char* argv[]) {
	t_log* logger = iniciar_logger("cpu.log","CPU",1,LOG_LEVEL_INFO);
	t_config* config = iniciar_config("cpu.config");
	get_config_info(logger, config);
	int dispatch_server = iniciar_servidor(info_config.puerto_escucha_dispath);
	int interrupt_server = iniciar_servidor(info_config.puerto_escucha_interrupt);
	
	log_info(logger, "Servers Loaded");
	//crear_conexion(info_config.ip_memoria, info_config.puerto_memoria);
	
	terminar_programa(dispatch_server, logger, config);

    return 0;
}

void terminar_programa(int conexion, t_log* logger, t_config* config) {
	log_destroy(logger);
	close(conexion);
	config_destroy(config);
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