#include "main.h"
#include "consola.h"
#include "kernel_interface_handler.h"
#include "planificador_corto_plazo.h"
#include "manejo_interrupciones_cpu.h"
#include "scheduler.h"
#include "semaforos.h"
#include "recursos.h"
#include "planificador_largo_plazo.h"

t_config* config;
t_kernel_config* kernel_config;
t_log* logger;
t_log* extra_logger;
int conexion_memoria, conexion_dispatch, conexion_interrupt, kernel_server;
extern t_dictionary *recursos;

int main(int argc, char* argv[]) {

    kernel_config = load_kernel_config("kernel.config");

    logger = iniciar_logger("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);

    extra_logger = iniciar_logger("kernel_debug.log", "KERNEL", 1, LOG_LEVEL_DEBUG);

    init_scheduler();

    init_semaforos();

    init_recursos();

    // El Kernel intenta conectarse con la memoria
    conexion_memoria = conectarse_a_modulo("MEMORIA", kernel_config->ip_memoria, kernel_config->puerto_memoria, KERNEL_CON_MEMORIA, extra_logger);

    // El Kernel intenta conectarse con la CPU en el puerto Dispatch
    conexion_dispatch = conectarse_a_modulo("CPU (PUERTO DISPATCH)", kernel_config->ip_cpu, kernel_config->puerto_cpu_dispatch, KERNEL_CON_CPU_DISPATCH, extra_logger);

    // El Kernel intenta conectarse con la CPU en el puerto Interrupt
    conexion_interrupt = conectarse_a_modulo("CPU (PUERTO INTERRUPT)", kernel_config->ip_cpu, kernel_config->puerto_cpu_interrupt, KERNEL_CON_CPU_INTERRUPT, extra_logger);

    // El Kernel inicia un servidor que escucha por conexiones de las interfaces I/O
    kernel_server = escuchar_conexiones_de("INTERFACES I/O", kernel_config->puerto_escucha, extra_logger);

    // Acepto interfaces en un thread aparte asi no frena la ejecución del programa
    manejador_de_interfaces(kernel_server);

    // Inicio la planificación de largo plazo en un hilo a parte
    planificador_largo_plazo();

    // Inicia la planifcación de corto plazo en hilos a parte (dispatch e interrupt)
    dispatch_handler();
    interrupt_handler();

    //sleep(10);

    //cambiar_grado_multiprogramacion(4);

    consola_kernel();

    liberar_kernel();

    puts("\nCerrando Kernel...");

    exit(OK);
}

void liberar_kernel() {
    close(kernel_server);
    close(conexion_dispatch);
    close(conexion_interrupt);
    close(conexion_memoria);
    log_destroy(logger);
    log_destroy(extra_logger);
    config_destroy(config);
    kernel_config_destroy();
}

t_kernel_config* load_kernel_config(String path) {

    config = iniciar_config(path);

    if(config == NULL) {
        fprintf(stderr, "Config invalido!\n");
        exit(EXIT_FAILURE);
    }

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

void kernel_config_destroy() {

    if(kernel_config == NULL)
        return;

    for (int i = 0; kernel_config->recursos[i] != NULL; i++)
        free(kernel_config->recursos[i]);

    free(kernel_config->recursos);

    for (int i = 0; kernel_config->instancias_recursos[i] != NULL; i++)
        free(kernel_config->instancias_recursos[i]);

    free(kernel_config->instancias_recursos);

    free(kernel_config);
}
