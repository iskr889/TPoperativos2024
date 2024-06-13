#include "kernel_comunicacion.h"

void enviar_io_gen_sleep(int socket, uint32_t tiempo) {
    payload_t *payload = payload_create(sizeof(uint32_t));
    payload_add(payload, &tiempo, sizeof(uint32_t));

    paquete_t *paquete = crear_paquete(IO_GEN_SLEEP, payload);

    if (enviar_paquete(socket, paquete) != OK) {
        perror("Error al enviar IO_GEN_SLEEP al kernel");
        exit(EXIT_FAILURE);
    }

    payload_destroy(payload);
    liberar_paquete(paquete);
}

uint32_t recibir_io_gen_sleep(int socket) {
    paquete_t *paquete = recibir_paquete(socket);
    if (paquete == NULL) {
        perror("Error al recibir IO_GEN_SLEEP del kernel");
        exit(EXIT_FAILURE);
    }

    uint32_t tiempo;
    payload_read(paquete->payload, &tiempo, sizeof(uint32_t));

    payload_destroy(paquete->payload);
    liberar_paquete(paquete);

    return tiempo;
}