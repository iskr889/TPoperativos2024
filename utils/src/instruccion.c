#include "instruccion.h"

payload_t *instruccion_serializar(char *instruccion) {
    uint32_t instruccion_length = strlen(instruccion) + 1;
    uint32_t total_size =  sizeof(uint32_t) + instruccion_length;
    payload_t *payload = payload_create(total_size);
    payload_add_string(payload, instruccion);
    return payload;
}

char *instruccion_deserializar(payload_t *payload) {
    payload->offset = 0;
    return payload_read_string(payload);
}

void enviar_instruccion(int socket, char *instruccion) {
    payload_t *payload = instruccion_serializar(instruccion);
    paquete_t *paquete = crear_paquete(1, payload);
    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);
    payload_destroy(payload);
    liberar_paquete(paquete);
}

char *recibir_instruccion(int socket) {
    paquete_t *paquete = recibir_paquete(socket);
    if(paquete == NULL)
        exit(EXIT_FAILURE);
    char *instruccion = instruccion_deserializar(paquete->payload);
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
    return instruccion;
}


void solicitar_intruccion(int socket, int *data) {
    payload_t *payload = serializar_solicitud(data);
    paquete_t *paquete = crear_paquete(2, payload);
    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);
    payload_destroy(payload);
    liberar_paquete(paquete);
}



payload_t *serializar_solicitud(int *data) {
    uint32_t total_size = sizeof(uint32_t) + 2 * sizeof(int);
    payload_t *payload = payload_create(total_size);
    payload_add_int_array(payload, data, 2);
    return payload;
}

int *deserializar_solicitud(payload_t *payload, uint32_t *count) {
    payload->offset = 0;
    return payload_read_int_array(payload, count);
}

int *recibir_solicitud_intruccion(int socket, uint32_t *count) {
    paquete_t *paquete = recibir_paquete(socket);
    if(paquete == NULL)
        exit(EXIT_FAILURE);
    int *data = deserializar_solicitud(paquete->payload, count);
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
    return data;
}