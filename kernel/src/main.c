#include "main.h"
#include "consola.h"
#include "kernel_interface_handler.h"

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
    t_config* config = iniciar_config("kernel.config");
    t_kernel_config* kernel_config = load_kernel_config(config);

    log_info(logger, "Archivo de configuración cargado correctamente");

    // El Kernel intenta conectarse con la memoria
    int conexion_memoria = conectarse_a_modulo("MEMORIA", kernel_config->ip_memoria, kernel_config->puerto_memoria, KERNEL_CON_MEMORIA, logger);

    // El Kernel intenta conectarse con la CPU en el puerto Dispatch
    int conexion_dispatch = conectarse_a_modulo("CPU (PUERTO DISPATCH)", kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch, KERNEL_CON_CPU_DISPATCH, logger);

    // El Kernel intenta conectarse con la CPU en el puerto Interrupt
    int conexion_interrupt = conectarse_a_modulo("CPU (PUERTO INTERRUPT)", kernel_config->ip_cpu, kernel_config->puerto_cpu_interrupt, KERNEL_CON_CPU_INTERRUPT, logger);

    // El Kernel inicia un servidor que escucha por conexiones de las interfaces I/O
    int kernel_server = escuchar_conexiones_de("INTERFACES I/O", kernel_config->puerto_escucha, logger);

    // Acepto interfaces en un thread aparte asi no frena la ejecución del programa
    manejador_de_interfaces(kernel_server);

    consola_kernel();

    // pthread_join(thread_interfaces, NULL); // Espero a que el thread creado termine

    // Cierro todos lo archivos y libero los punteros usados
    close(kernel_server);
    close(conexion_dispatch);
    close(conexion_interrupt);
    close(conexion_memoria);
    log_destroy(logger);
    config_destroy(config);
    kernel_config_destroy(kernel_config); // Libera todos los punteros de la estructura kernel_config

    puts("Cerrando Kernel...");

    exit(OK);
}

t_kernel_config* load_kernel_config(t_config* config) {

    t_kernel_config* kernel_config = malloc(sizeof(t_kernel_config));
    
    if (kernel_config == NULL) {
        perror("Error en malloc()");
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
