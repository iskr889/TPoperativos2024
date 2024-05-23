#include "main.h"

t_cpu_config cpu_config;

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("cpu.log", "CPU", 1, LOG_LEVEL_INFO);
    t_config* config = iniciar_config("cpu.config");
    load_cpu_config(config);

    log_info(logger, "Archivo de configuración cargado correctamente");

    // El Kernel intenta conectarse con la memoria
    int fd_memoria = conectarse_a_modulo("MEMORIA", cpu_config.ip_memoria, cpu_config.puerto_memoria, CPU_CON_MEMORIA, logger);

    // La CPU inicia un servidor que escucha por conexiones del Kernel a la CPU (DISPATCH)
    int dispatch_server = modulo_escucha_conexiones_de("KERNEL (PUERTO DISPATCH)", cpu_config.puerto_escucha_dispath, logger);
    // La CPU inicia un servidor que escucha por conexiones del Kernel a la CPU (INTERRUPT)
    int interrupt_server = modulo_escucha_conexiones_de("KERNEL (PUERTO INTERRUPT)", cpu_config.puerto_escucha_interrupt, logger);

    conexionId_t* conexion_dispatch = esperar_conexion_de(KERNEL_CON_CPU_DISPATCH, dispatch_server);
    conexionId_t* conexion_interrupt = esperar_conexion_de(KERNEL_CON_CPU_INTERRUPT, interrupt_server);

    // Cierro todos lo archivos y libero los punteros usados
    close(fd_memoria);
    close(dispatch_server);
    close(interrupt_server);
    log_destroy(logger);
    config_destroy(config);

    return OK;
}

void load_cpu_config(t_config* config) {
    cpu_config.ip_memoria = config_get_string_value(config,"IP_MEMORIA");
    cpu_config.puerto_memoria = config_get_string_value(config,"PUERTO_MEMORIA");
    cpu_config.puerto_escucha_dispath = config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH");
    cpu_config.puerto_escucha_interrupt = config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT");
    cpu_config.cantidad_entradas = config_get_int_value(config,"CANTIDAD_ENTRADAS_TLB");
    cpu_config.algoritmo_tlb = config_get_string_value(config,"ALGORITMO_TLB");
}