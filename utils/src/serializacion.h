#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

#include "utils.h"

typedef struct {
    uint32_t size;   // Tamaño del payload
    uint32_t offset; // Desplazamiento dentro del payload
    void* stream;    // Payload
} payload_t;

typedef struct {
	int operacion;
	payload_t* payload;
} paquete_t;

// Solo recibe la operación a ejecutar
int recibir_operacion(int socket);

// Solo envia la operación a ejecutar
void enviar_operacion(int operacion, int socket);

// Crea un payload vacío de tamaño size y offset 0
payload_t *payload_create(uint32_t size);

// Libera la memoria asociada al payload
void payload_destroy(payload_t *payload);

// Agrega un stream al payload en la posición actual y avanza el offset
void payload_add(payload_t *payload, void *data, uint32_t size);

// Guarda size bytes del principio del payload en la dirección data y avanza el offset
void payload_read(payload_t *payload, void *data, uint32_t size);

// Agrega string al payload con un uint32_t adelante indicando su longitud
void payload_add_string(payload_t *payload, String string);

// Lee un string y su longitud del payload y avanza el offset
String payload_read_string(payload_t *payload);

// Crea un paquete con el codigo de operación a enviar
paquete_t *crear_paquete(int codigo_operacion, payload_t *buffer);

// Libera el paquete creado
void liberar_paquete(paquete_t *paquete);

// Envia un paquete al socket indicado
int enviar_paquete(int socket, paquete_t *paquete);

// Recibe el paquete del socket indicado
paquete_t *recibir_paquete(int socket);

void payload_add_int_array(payload_t *payload, int *array, uint32_t count);
int* payload_read_int_array(payload_t *payload, uint32_t *count);

#endif
