#include "serializacion.h"

int recibir_operacion(int socket) {
    int operacion;
    if (recv(socket, &operacion, sizeof(operacion), MSG_WAITALL) <= 0) {
        perror("Error al recibir codigo de operacion");
        close(socket);
        return -1;
    }
    return operacion;
}

void enviar_operacion(int operacion, int socket) {
	if (send(socket, &operacion, sizeof(operacion), 0) < 0) {
		perror("Error al tratar de enviar el codigo de operacion");
        close(socket);
        exit(EXIT_FAILURE);
	}
}

payload_t *payload_create(uint32_t size) {
    payload_t *payload = malloc(sizeof(payload_t));
    payload->size = size;
    payload->offset = 0;
    payload->stream = malloc(size);
    return payload;
}

void payload_destroy(payload_t *payload) {
    free(payload->stream);
    free(payload);
}

void payload_add(payload_t *payload, void *data, uint32_t size) {
    memcpy(payload->stream + payload->offset, data, size);
    payload->offset += size;
}

void payload_read(payload_t *payload, void *data, uint32_t size) {
    memcpy(data, payload->stream + payload->offset, size);
    payload->offset += size;
}

void payload_add_string(payload_t *payload, String string) {
    uint32_t length = strlen(string) + 1; // Tengo en cuenta el '\0'
    payload_add(payload, &length, sizeof(length));
    payload_add(payload, string, length);
}

String payload_read_string(payload_t *payload) {
    uint32_t length;
    payload_read(payload, &length, sizeof(length));
    String string = malloc(length);
    payload_read(payload, string, length);
    return string;
}

paquete_t *crear_paquete(int codigo_operacion, payload_t *payload) {
    paquete_t *paquete = malloc(sizeof(paquete_t));
    paquete->operacion = codigo_operacion;
    paquete->payload = payload;
    return paquete;
}

void liberar_paquete(paquete_t *paquete) {
	free(paquete);
}

int enviar_paquete(int socket, paquete_t *paquete) {
    // Calcular el tamaño total del paquete
    uint32_t size = sizeof(paquete->operacion) + sizeof(paquete->payload->size) + paquete->payload->size;

    void *data = malloc(size);
    int offset = 0;

    memcpy(data + offset, &paquete->operacion, sizeof(paquete->operacion));
    offset += sizeof(paquete->operacion);
    memcpy(data + offset, &paquete->payload->size, sizeof(paquete->payload->size));
    offset += sizeof(paquete->payload->size);
    memcpy(data + offset, paquete->payload->stream, paquete->payload->size);

    if(send(socket, data, size, 0) <= 0) {
		perror("Error al enviar paquete");
		free(data);
		return ERROR;
	}

    free(data);

	return OK;
}

paquete_t *recibir_paquete(int socket) {
    // Recibir el código de operación
    int codigo_operacion;
    if(recv(socket, &codigo_operacion, sizeof(codigo_operacion), 0) <= 0) {
		perror("Error al recibir codigo de operacion");
		return NULL;
	}

    uint32_t payload_size;
    if(recv(socket, &payload_size, sizeof(payload_size), 0) <= 0) {
		perror("Error al recibir payload size");
		return NULL;
	}

    payload_t *payload = payload_create(payload_size);
    if(recv(socket, payload->stream, payload_size, 0) <= 0) {
		perror("Error al recibir el payload");
		return NULL;
	}

    paquete_t *paquete = crear_paquete(codigo_operacion, payload);

    return paquete;
}
