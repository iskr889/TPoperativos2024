#ifndef UTILS_INTRUCCION_H_
#define UTILS_INTRUCCION_H_

#include "utils.h"
#include "serializacion.h"

payload_t *instruccion_serializar(char *instruccion);
char *instruccion_deserializar(payload_t *payload);
void enviar_instruccion(int socket, char *instruccion);
char *recibir_instruccion(int socket);

void enviar_pc(uint32_t pc);

typedef enum {
    I_SET,
    I_SUM,
    I_SUB,
    I_JNZ,
    I_IO_GEN_SLEEP,
    I_MOV_IN,
    I_MOV_OUT,
    I_RESIZE,
    I_COPY_STRING,
    I_WAIT,
    I_SIGNAL,
    I_IO_STDIN_READ,
    I_IO_STDOUT_WRITE,
    I_IO_FS_CREATE,
    I_IO_FS_DELETE,
    I_IO_FS_TRUNCATE,
    I_IO_FS_WRITE,
    I_IO_FS_READ,
    I_EXIT,
} tipo_instruccion_t;

#endif