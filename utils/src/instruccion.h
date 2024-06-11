#ifndef UTILS_INTRUCCION_H_
#define UTILS_INTRUCCION_H_

#include "utils.h"
#include "serializacion.h"


instruccion_t* pedir_instruccion(uint32_t pc);
void enviar_pc(uint32_t pc);
void* serializar_instruccion(instruccion_t* instruccion, int* size);
payload_t *instruccion_serializar(instruccion_t *instruccion);
instruccion_t *instruccion_deserializar(payload_t *payload);
void enviar_instruccion(int socket, instruccion_t *instruccion);
instruccion_t *recibir_instruccion(int socket);
instruccion_t *crear_instruccion(tipo_instruccion_t tipo, registro_t reg1, registro_t reg2, uint32_t valor, const char* interfaz, const char* recurso, const char* nombreArchivo);
void imprimir_instruccion(instruccion_t *instruccion);

#endif