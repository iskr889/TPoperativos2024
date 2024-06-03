#include "main.h"

t_config *config;
t_interfaz_config* interfaz_config;
t_log *logger;
t_log *extra_logger;

int main(int argc, char *argv[]) {

    if (argc != 3) {
        log_error(extra_logger, "Cantidad de argumentos del programa incorrectos!");
        return ERROR;
    }

    interfaz_config = load_io_config(argv[2]);

    logger = iniciar_logger("entradasalida.log", argv[1], 1, LOG_LEVEL_INFO);

    extra_logger = iniciar_logger("entradasalida_debug.log", argv[1], 1, LOG_LEVEL_DEBUG);

    String interfaz = interfaz_config->tipo_interfaz;

    if (interfaz == NULL) {
        log_error(extra_logger, "Error al leer la interfaz del archivo config!");
        liberar_interfaz();
        return ERROR;
    } else if (strcmp(interfaz, "GENERIC") == 0) {
        interfaz_generica(argv[1]);
    } else if (strcmp(interfaz, "STDIN") == 0) {
        interfaz_stdin(argv[1]);
    } else if (strcmp(interfaz, "STDOUT") == 0) {
        interfaz_stdout(argv[1]);
    } else if (strcmp(interfaz, "DialFS") == 0) {
        interfaz_dialFS(argv[1]);
    } else {
        log_error(extra_logger, "Nombre de interfaz invalido!");
        liberar_interfaz();
        return ERROR;
    }

    liberar_interfaz();

    puts("\nCerrando Interfaz...");

    return OK;
}

void liberar_interfaz() {
    log_destroy(logger);
    log_destroy(extra_logger);
    config_destroy(config);
    io_config_destroy();
}

t_interfaz_config* load_io_config(String path) {

    config = iniciar_config(path);

    if(config == NULL) {
        fprintf(stderr, "Config invalido!\n");
        exit(EXIT_FAILURE);
    }

    interfaz_config = malloc(sizeof(t_interfaz_config));

    if(interfaz_config == NULL) {
        perror("Fallo malloc");
        exit(EXIT_FAILURE);
    }

    interfaz_config->tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    interfaz_config->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    interfaz_config->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

    interfaz_config->ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    interfaz_config->puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");

    interfaz_config->path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");

    String tiempo_unidad_trabajo = config_get_string_value(config, "TIEMPO_UNIDAD_TRABAJO");

    if (tiempo_unidad_trabajo != NULL)
        interfaz_config->tiempo_u_trabajo = atoi(tiempo_unidad_trabajo);
    else
        interfaz_config->tiempo_u_trabajo = 0;

    String block_size = config_get_string_value(config, "BLOCK_SIZE");

    if (block_size != NULL)
        interfaz_config->block_size = atoi(block_size);
    else
        interfaz_config->block_size = 0;

    String block_count = config_get_string_value(config, "BLOCK_COUNT");

    if (block_count != NULL)
        interfaz_config->block_count = atoi(block_count);
    else
        interfaz_config->block_count = 0;

    String retraso_compactacion = config_get_string_value(config, "RETRASO_COMPACTACION");

    if (retraso_compactacion != NULL)
        interfaz_config->retraso_compactacion = atoi(retraso_compactacion);
    else
        interfaz_config->retraso_compactacion = 0;

    return interfaz_config;
}

void io_config_destroy() {
    if(interfaz_config == NULL)
        return;
    free(interfaz_config->tipo_interfaz);
    free(interfaz_config->ip_kernel);
    free(interfaz_config->ip_memoria);
    free(interfaz_config->puerto_memoria);
    free(interfaz_config->puerto_kernel);
    free(interfaz_config->path_base_dialfs);
    free(interfaz_config);
}

void io_gen_sleep(int unidades_trabajo) {
    if (unidades_trabajo <= 0) {
        log_error(extra_logger, "Unidades de trabajo invalidas!");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Operacion: IO_GEN_SLEEP %dms", unidades_trabajo);
    TIEMPO_UNIDAD_DE_TRABAJO(unidades_trabajo);
}

void enviar_nombre_interfaz(String nombre, int socket) {

    payload_t *payload = payload_create(sizeof(uint32_t) + strlen(nombre) + 1);

    payload_add_string(payload, nombre);

    paquete_t *paquete = crear_paquete(0, payload); // El op code no importa en este caso

    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);

    payload_destroy(payload);
    liberar_paquete(paquete);
}

void interfaz_generica(String nombre) {

    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, GENERIC_CON_KERNEL, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while(1)
        generic_procesar_instrucciones(conexion_kernel);

    close(conexion_kernel);
}

void generic_procesar_instrucciones(int socket) {
    paquete_t *paquete = recibir_paquete(socket);

    if(paquete == NULL)
        exit(EXIT_FAILURE);

    if(paquete->operacion != IO_GEN_SLEEP) {
        log_error(extra_logger, "Instruccion recibida por el kernel no reconocida!");
        return;
    }

    uint32_t tiempo_sleep;

    payload_read(paquete->payload, &tiempo_sleep, sizeof(uint32_t));

    payload_destroy(paquete->payload);
    liberar_paquete(paquete);

    io_gen_sleep(tiempo_sleep);
}

void interfaz_stdin(String nombre) {
    return;
}

void interfaz_stdout(String nombre) {
    return;
}

void interfaz_dialFS(String nombre) {
    return;
}
