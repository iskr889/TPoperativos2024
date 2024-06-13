#include "instruccion.h"

payload_t *instruccion_serializar(char *instruccion) {
    uint32_t instruccion_length = strlen(instruccion) + 1;
    uint32_t total_size =  sizeof(uint32_t) + instruccion_length;
    payload_t *payload = payload_create(total_size);

    payload_add(payload, &instruccion_length, sizeof(uint32_t));
    payload_add(payload, instruccion, instruccion_length);

    return payload;
}

char *instruccion_deserializar(payload_t *payload) {
    payload->offset = 0;
    uint32_t length;
    payload_read(payload, &length, sizeof(uint32_t));
    char * instruccion = malloc(length);
    payload_read(payload, instruccion, length);
    return instruccion;
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