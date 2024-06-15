#ifndef I_CPU_H
#define I_CPU_H

#include "main.h"

typedef enum {
    SUMA,
    RESTA,
    ASIGNACION
} operacion_t;

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

int getTipoRegistro(char *tipo);
int obtener_tipo_instruccion(const char* tipo_str);
char** split_string(char* str);
void actualizar_registro(cpu_reg_t* registros, registro_t registro, uint32_t valor, operacion_t operacion);
char* fetch(pcb_t* pcb);
void decode(char* instruccion, pcb_t* pcb);
void execute(char* instruccion, pcb_t* pcb);
void* obtener_registro(cpu_reg_t* registros, registro_t registro);

char *recibir_instruccion(int socket);

void solicitar_intruccion(int socket, uint16_t pid, uint32_t pc);

// uint32_t obtener_valor_memoria(uint32_t direccion) {
// }

// void almacenar_en_memoria(uint32_t direccion, uint32_t valor) {
// }

// void resize_recurso(char* recurso, uint32_t nuevo_tamano) {
// }

// char* obtener_cadena_de_registro(cpu_reg_t* registros, registro_t registro) {
// }

// void copiar_a_registro(cpu_reg_t* registros, registro_t registro, char* string) {
// }

// void wait_recurso(char* recurso) {
// }

// void signal_recurso(char* recurso) {
// }

// uint32_t leer_stdin() {
// }

#endif