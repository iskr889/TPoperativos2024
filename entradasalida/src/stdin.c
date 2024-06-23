#include "stdin.h"

extern t_interfaz_config* interfaz_config;
extern t_log *logger;
extern t_log *extra_logger;

void interfaz_stdin(String nombre) {

    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, STDIN_CON_KERNEL, extra_logger);

    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, STDIN_CON_MEMORIA, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while(1){
            stdin_procesar_instrucciones(conexion_kernel, conexion_memoria);
            enviar_operacion(OK, conexion_kernel);
        } 
        
    close(conexion_kernel);
    close(conexion_memoria);
}

String leer_texto(int cant_caracteres) {
    while (1) {
        String texto = readline(">> Ingresar texto para guardar en memoria: ");
        if (texto != NULL && strlen(texto) == cant_caracteres)
            return texto;
        free(texto);
        log_error(extra_logger, "El texto ingresado es invalido, volver a ingresar...");
    }
}

bool stdin_procesar_instrucciones(int fd_kernel, int fd_memoria) {

    paquete_t *paquete = recibir_paquete(fd_kernel);

    if (paquete == NULL || paquete->operacion != IO_STDIN_READ) {
        log_error(extra_logger, "Instrucción recibida del kernel invalida!");
        return false;
    }

    uint32_t direccion, cant_caracteres;
    payload_read(paquete->payload, &direccion, sizeof(uint32_t));
    payload_read(paquete->payload, &cant_caracteres, sizeof(uint32_t));

    String texto = leer_texto(cant_caracteres);

    payload_t *payload_a_enviar = payload_create(sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t) + cant_caracteres);

    char operacion = 'W'; // Necesario para escribir memoria en espacio de usuario
    payload_add(payload_a_enviar, &operacion, sizeof(char));
    payload_add(payload_a_enviar, &direccion, sizeof(uint32_t));
    payload_add(payload_a_enviar, &cant_caracteres, sizeof(uint32_t));
    payload_add(payload_a_enviar, texto, cant_caracteres);

    paquete_t *paquete_a_enviar = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload_a_enviar);

    if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK) {
        log_error(extra_logger, "No se pudo enviar el paquete a la memoria!");
        return false;
    }

    payload_destroy(paquete->payload);
    liberar_paquete(paquete);

    payload_destroy(payload_a_enviar);
    liberar_paquete(paquete_a_enviar);

    free(texto);

    // Debajo recibo la respuesta de la memoria

    paquete_t *respuesta = recibir_paquete(fd_memoria);

    if(respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
        log_error(extra_logger, "Error al recibir respuesta de la memoria\n");
        return false;
    }

    String rta_memoria = payload_read_string(respuesta->payload);

    log_info(extra_logger, "Memoria respondio: %s", rta_memoria);

    bool escritura_exitosa = strcmp(rta_memoria, "OK") == 0 ? true : false;

    free(rta_memoria);

    payload_destroy(respuesta->payload);
    liberar_paquete(respuesta);

    return escritura_exitosa;
}
