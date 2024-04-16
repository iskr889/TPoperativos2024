#include "main.h"

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger();

    t_config* config = iniciar_config();

    t_kernel_config* kernel_config = load_kernel_config(config);
    
    //inicializar kernel

    //iniciar servidor de kernel
	int fd_kernel = iniciar_servidor(kernel_config->puerto_escucha, logger, "KERNEL");
	
	//conectarme con cpu en modo dispatch
	int fd_cpu_dispatch = crear_conexion(kernel_config->ip_cpu ,kernel_config->puerto_cpu_dispatch, logger, "CPU DISPATCH");
    log_info(logger, "[DISPATCH] CONECTADO a CPU");

    //conectarme con cpu en modo interrupt
    int fd_cpu_interrupt = crear_conexion(kernel_config->ip_cpu ,kernel_config->puerto_cpu_interrupt, logger, "CPU INTERRUPT");
    log_info(logger, "[INTERRUPT] CONECTADO a CPU");

	//conectarme con memoria
    int fd_memoria = crear_conexion(kernel_config->ip_memoria, kernel_config->puerto_memoria, logger, "MEMORIA");
    log_info(logger, "CONECTADO a MEMORIA");

    // esperar conexion de entradaSalida
    log_info(logger, "[KERNEL] esperando conexion de ENTRADA/SALIDA...");
    int fd_estradaSalida = esperar_cliente(fd_kernel, logger, "ENTRADA/SALIDA");

    // Termino todo y libero los punteros usados

    config_destroy(config); // Libera la memoria de config

    free(kernel_config);

    return 0;
}


t_config* iniciar_config(void) {
	
    t_config* nuevo_config = config_create("kernel.config");
	
    if(nuevo_config == NULL)
        error_exit("Error al tratar de crear kernel.config");
	
    return nuevo_config;
}


t_log* iniciar_logger(void) {
	
    t_log* nuevo_logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
	
    if(nuevo_logger == NULL)
        error_exit("Error al tratar de crear kernel.log");
	
    return nuevo_logger;
}


void error_exit(char *message) { 
    perror(message);
    exit(EXIT_FAILURE);
}

t_kernel_config* load_kernel_config(t_config* config) { // Liberar kernel_config despues de usar

    t_kernel_config* kernel_config = malloc(sizeof(t_kernel_config));
    
    if(kernel_config == NULL) {
        perror("Fallo malloc");
        exit(EXIT_FAILURE);
    }
    
    kernel_config->puerto_escucha = config_get_int_value(config, "PUERTO_ESCUCHA");

    kernel_config->ip_memoria = config_get_string_value(config,"IP_MEMORIA");

    kernel_config->puerto_memoria = config_get_int_value(config,"PUERTO_MEMORIA");

    kernel_config->ip_cpu = config_get_string_value(config,"IP_CPU");

    kernel_config->puerto_cpu_dispatch = config_get_int_value(config,"PUERTO_CPU_DISPATCH");
    
    kernel_config->puerto_cpu_interrupt = config_get_int_value(config,"PUERTO_CPU_INTERRUPT");

    kernel_config->algoritmo_planificacion = config_get_string_value(config,"ALGORITMO_PLANIFICACION");

    kernel_config->quantum = config_get_int_value(config,"QUANTUM");

    kernel_config->recursos = config_get_array_value(config,"RECURSOS");

    kernel_config->instancias_recursos = config_get_array_value(config,"INSTANCIAS_RECURSOS");

    kernel_config->grado_multiprogramacion = config_get_int_value(config,"GRADO_MULTIPROGRAMACION");

    return kernel_config;
}