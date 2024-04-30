#include "main.h"

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("kernel.log", "KERNEL",1,LOG_LEVEL_INFO);

    t_config* config = iniciar_config("kernel.config");

    t_kernel_config* kernel_config = load_kernel_config(config);
    
    //inicializar kernel

    // Completar

    //iniciar servidor de kernel
	int fd_kernel_server = iniciar_servidor(kernel_config->puerto_escucha);

    if(fd_kernel_server < 0)
        return EXIT_ERROR;
    
    log_info(logger, "[KERNEL] SERVIDOR INICIADO");

    pthread_t thread_id;

    // Acepto clientes en un thread aparte asi no frena la ejecución del programa
    if(pthread_create(&thread_id, NULL, thread_aceptar_clientes, &fd_kernel_server) != 0) {
        perror("No se pudo crear el hilo");
        return EXIT_ERROR;
    }

	//conectarme con cpu en modo dispatch
	int fd_cpu_dispatch = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch);
    log_info(logger, "[KERNEL] CONECTADO A CPU (DISPATCH)");

    //conectarme con cpu en modo interrupt
    int fd_cpu_interrupt = crear_conexion(kernel_config->ip_cpu, kernel_config->puerto_cpu_interrupt);
    log_info(logger, "[KERNEL] CONECTADO A CPU (INTERRUPT)");

	//conectarme con memoria
    int fd_memoria = crear_conexion(kernel_config->ip_memoria, kernel_config->puerto_memoria);
    log_info(logger, "[KERNEL] CONECTADO A MEMORIA");

    // esperar conexion de entradaSalida
    // log_info(logger, "[KERNEL] esperando conexion de ENTRADA/SALIDA...");
    // int fd_estradaSalida = esperar_cliente(fd_kernel, logger, "ENTRADA/SALIDA");

    // Termino todo y libero los punteros usados
    close(fd_kernel_server);
    close(fd_cpu_dispatch);
    close(fd_cpu_interrupt);
    close(fd_memoria);

    log_destroy(logger);

    config_destroy(config); // Libera la memoria de config

    kernel_config_destroy(kernel_config);

    return 0;
}

t_kernel_config* load_kernel_config(t_config* config) {

    t_kernel_config* kernel_config = malloc(sizeof(t_kernel_config));
    
    if(kernel_config == NULL) {
        perror("Fallo malloc");
        exit(EXIT_FAILURE);
    }
    
    kernel_config->puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");

    kernel_config->ip_memoria = config_get_string_value(config,"IP_MEMORIA");

    kernel_config->puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");

    kernel_config->ip_cpu = config_get_string_value(config,"IP_CPU");

    kernel_config->puerto_cpu_dispatch = config_get_string_value(config,"PUERTO_CPU_DISPATCH");
    
    kernel_config->puerto_cpu_interrupt = config_get_string_value(config,"PUERTO_CPU_INTERRUPT");

    kernel_config->algoritmo_planificacion = config_get_string_value(config,"ALGORITMO_PLANIFICACION");

    kernel_config->quantum = config_get_int_value(config,"QUANTUM");

    kernel_config->recursos = config_get_array_value(config,"RECURSOS");

    kernel_config->instancias_recursos = config_get_array_value(config,"INSTANCIAS_RECURSOS");

    kernel_config->grado_multiprogramacion = config_get_int_value(config,"GRADO_MULTIPROGRAMACION");

    return kernel_config;
}

void kernel_config_destroy(t_kernel_config* kernel_config) {

    for (int i = 0; kernel_config->recursos[i] != NULL; i++)
        free(kernel_config->recursos[i]);

    free(kernel_config->recursos);

    for (int i = 0; kernel_config->instancias_recursos[i] != NULL; i++)
        free(kernel_config->instancias_recursos[i]);

    free(kernel_config->instancias_recursos);

    free(kernel_config);

    return;
}