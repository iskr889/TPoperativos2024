#include "stdout.h"

extern t_interfaz_config* interfaz_config;
extern t_log *logger;
extern t_log *extra_logger;


void interfaz_stdout(String nombre) {

    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, STDOUT_CON_KERNEL, extra_logger);

    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, STDOUT_CON_MEMORIA, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while(1){
            stdout_procesar_instrucciones(conexion_kernel, conexion_memoria);
            enviar_operacion(OK, conexion_kernel);
    }

    close(conexion_kernel);
    close(conexion_memoria);
}

bool stdout_procesar_instrucciones(int fd_kernel, int fd_memoria) {

    paquete_t *paquete = recibir_paquete(fd_kernel);

    if (paquete == NULL || paquete->operacion != IO_STDOUT_WRITE) {
        log_error(extra_logger, "Instrucción recibida del kernel invalida!");
        return false;
    }

    uint16_t pid;
    uint32_t direccion, cant_caracteres;
    payload_read(paquete->payload, &pid, sizeof(uint16_t));
    payload_read(paquete->payload, &direccion, sizeof(uint32_t));
    payload_read(paquete->payload, &cant_caracteres, sizeof(uint32_t));

    payload_t *payload_a_enviar = payload_create(sizeof(char) + sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint32_t));

    char operacion = 'R'; // Necesario para leer memoria en espacio de usuario
    payload_add(payload_a_enviar, &pid, sizeof(uint16_t));
    payload_add(payload_a_enviar, &operacion, sizeof(char));
    payload_add(payload_a_enviar, &direccion, sizeof(uint32_t));
    payload_add(payload_a_enviar, &cant_caracteres, sizeof(uint32_t));

    paquete_t *paquete_a_enviar = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload_a_enviar);

    if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK) {
        log_error(extra_logger, "No se pudo enviar el paquete a la memoria!");
        return false;
    }

    log_info(logger, "PID: %d - Operacion: IO_STDOUT_WRITE", pid); // LOG OBLIGATORIO

    payload_destroy(paquete->payload);
    liberar_paquete(paquete);

    payload_destroy(payload_a_enviar);
    liberar_paquete(paquete_a_enviar);

    // Debajo recibo la respuesta de la memoria

    paquete_t *respuesta = recibir_paquete(fd_memoria);

    if(respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
        log_error(extra_logger, "Error al recibir respuesta de la memoria\n");
        return false;
    }

    char *buffer_stdout = malloc(cant_caracteres);

    payload_read(respuesta->payload, buffer_stdout, cant_caracteres);

    buffer_stdout[cant_caracteres] = '\0'; // Por las dudas

    log_info(extra_logger, "STDOUT: %s", buffer_stdout);

    payload_destroy(respuesta->payload);
    liberar_paquete(respuesta);

    return true;
}
