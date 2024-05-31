#include "main.h"

t_log *logger = NULL;
t_config *config = NULL;
t_interfaz_config* interfaz_config = NULL;

int main(int argc, char *argv[]) {

    if (argc != 3) {
        log_error(logger, "Cantidad de argumentos del programa incorrectos!");
        return ERROR;
    }

    config = iniciar_config(argv[2]);

    interfaz_config = load_io_config();

    logger = iniciar_logger("entradasalida.log", argv[1], 1, LOG_LEVEL_INFO);

    String interfaz = interfaz_config->tipo_interfaz;

    if (interfaz == NULL) {
        log_error(logger, "Error al leer la interfaz del archivo config!");
        liberar_todo();
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
        log_error(logger, "Nombre de interfaz invalido!");
        liberar_todo();
        return ERROR;
    }

    liberar_todo();

    return OK;
}

void liberar_todo() {
    log_destroy(logger);
    config_destroy(config);
    io_config_destroy();
}

t_interfaz_config* load_io_config() {

    interfaz_config = malloc(sizeof(t_interfaz_config));

    if(interfaz_config == NULL) {
        perror("Fallo malloc");
        exit(EXIT_FAILURE);
    }

    if(config == NULL) {
        fprintf(stderr, "Config invalido!\n");
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
        log_error(logger, "Unidades de trabajo invalidas!");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Operacion: IO_GEN_SLEEP");
    usleep(unidades_trabajo * 1000);
}

t_instruccion_generica *recibir_instruccion(int socket_cliente) {
    t_instruccion_generica *instrucciones = malloc(sizeof(t_instruccion_generica));
    int tamanio;
    int u_trabajo;
    void *buffer;

    recv(socket_cliente, &tamanio, sizeof(int), MSG_WAITALL);
    instrucciones->instruccion = malloc(tamanio + 1);
    buffer = malloc(tamanio);
    recv(socket_cliente, buffer, tamanio, MSG_WAITALL);
    memcpy(instrucciones->instruccion, buffer, tamanio);
    instrucciones->instruccion[tamanio] = '\0';
    free(buffer);
    recv(socket_cliente, &u_trabajo, sizeof(int), MSG_WAITALL);
    instrucciones->u_trabajo = u_trabajo;
    return instrucciones;
}

int enviar_nombre(String nombre, int socket) {
    int len = strlen(nombre) + 1;
    if(send(socket, nombre, len, 0) != len) {
        perror("Error al tratar de enviar el nombre de la interfaz");
        return ERROR;
    }
    return OK;
}

void interfaz_generica(String nombre) {

    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, GENERIC_CON_KERNEL, logger);

    enviar_nombre(nombre, conexion_kernel);

    // while (1) {

    //     log_info(logger, ">> Esperando instruccion...");

    //     t_instruccion_generica *instruccion = recibir_instruccion(conexion_kernel);

    //     if (strcmp(instruccion->instruccion, "IO_GEN_SLEEP") == 0) {
    //         io_gen_sleep(instruccion->u_trabajo);
    //         free(instruccion->instruccion);
    //         free(instruccion);
    //     } else {
    //         log_error(logger, "La instruccion recibida no es valida!");
    //         close(conexion_kernel);
    //         free(instruccion->instruccion);
    //         free(instruccion);
    //         break;
    //     }
    // }
    close(conexion_kernel);
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
