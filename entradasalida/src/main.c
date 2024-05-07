#include "main.h"

// typedef enum {
//     STDOUT,
//     STDIN,
//     GENERIC,
//     DIALFS,
//     ERROR
// } interfaz_t ; 

int main(int argc, char* argv[]) {

    t_log* logger = iniciar_logger("entradasalida.log", "KERNEL", 1, LOG_LEVEL_INFO);
    t_config* config = iniciar_config("entradasalida.config");
    t_interfaz_config* interfaz_config = load_interfaz_config(config);

    log_info(logger, "Archivo de configuración cargado correctamente");

    // La interfaz intenta conectarse con la memoria
    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, GENERIC, logger);

    // La interfaz intenta conectarse con el kernel
    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, GENERIC, logger);

    close(conexion_kernel);
    close(conexion_memoria);
    log_destroy(logger);
    config_destroy(config);
    free(interfaz_config);

    return EXIT_OK;
}

t_interfaz_config* load_interfaz_config(t_config* config){
    t_interfaz_config* interfaz_config = malloc(sizeof(t_interfaz_config));

    if(interfaz_config == NULL) {
        perror("Fallo malloc");
        exit(EXIT_FAILURE);
    }

    interfaz_config->tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    interfaz_config->tiempo_u_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfaz_config->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    interfaz_config->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    interfaz_config->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    interfaz_config->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    interfaz_config->path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    interfaz_config->block_size = config_get_int_value(config, "BLOCK_SIZE");
    interfaz_config->block_count = config_get_int_value(config, "BLOCK_COUNT");

    return interfaz_config;
}
