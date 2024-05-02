#include "main.h"

info_config_struct info_config;

int main(int argc, char* argv[]) {
	t_log* logger = iniciar_logger("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
	t_config* config = iniciar_config("cpu.config");
	get_config_info(logger, config);

	// El Kernel inicia un servidor que escucha por conexiones de la CPU (DISPATCH)
	int dispatch_server = modulo_escucha_conexiones_de("CPU (DISPATCH)", info_config.puerto_escucha_dispath, logger);
	// El Kernel inicia un servidor que escucha por conexiones de la CPU (INTERRUPT)
	int interrupt_server = modulo_escucha_conexiones_de("CPU (INTERRUPT)", info_config.puerto_escucha_interrupt, logger);
	
	// Acepto clientes en un thread aparte asi no frena la ejecución del programa
    pthread_t thread_cpu_dispatch, thread_cpu_interrupt;
    atender_conexiones_al_modulo(&thread_cpu_dispatch, dispatch_server);
    atender_conexiones_al_modulo(&thread_cpu_interrupt, interrupt_server);

	// El Kernel intenta conectarse con la memoria
    int fd_memoria = conectarse_a_modulo("MEMORIA", info_config.ip_memoria, info_config.puerto_memoria, logger);

    sleep(10); // TODO: Borrar! Solo sirve para testear rapidamente la conexion entre modulos

    pthread_join(thread_cpu_dispatch, NULL);
	pthread_join(interrupt_server, NULL);

	close(fd_memoria);
	
	terminar_programa(dispatch_server, interrupt_server, logger, config);

    return EXIT_OK;
}

void terminar_programa(int dispatch_server, int interrupt_server, t_log* logger, t_config* config) {
	log_destroy(logger);
	close(dispatch_server);
	close(interrupt_server);
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