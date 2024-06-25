#ifndef I_CPU_H
#define I_CPU_H

#include "main.h"
#include "mmu.h"
#include "../../utils/src/interrupcion.h"

typedef enum {
    SUMA,
    RESTA,
    ASIGNACION
} operacion_t;

int getTipoRegistro(char *tipo);
int obtener_tipo_instruccion(const char* tipo_str);
char** split_string(char* str);
void actualizar_registro(cpu_reg_t* registros, registro_t registro, uint32_t valor, operacion_t operacion);
char* fetch(pcb_t* pcb);
char* decode(char* instruccion, pcb_t* pcb);
void execute(char* instruccion, pcb_t* pcb);
void* obtener_registro(cpu_reg_t* registros, registro_t registro);

char *recibir_instruccion(int socket);

void solicitar_intruccion(int socket, uint16_t pid, uint32_t pc);
void i_io_fs_operation(char *t_instruccion, char *interfaz, char *nombre_archivo, char *direccion, char *tamanio, char *puntero_archivo, uint16_t cod, pcb_t *pcb);
void i_io_generic_operation(char *t_instruccion, char *interfaz, char *tiempo_sleep, uint16_t cod, pcb_t *pcb);
void i_io_stdin_operation(char *t_instruccion, char *interfaz, char *direccion, char *tamanio, uint16_t cod, pcb_t *pcb);
bool leer_memoria(uint32_t direccion_fisica, void* buffer, uint32_t size);
bool escribir_memoria(uint32_t direccion_fisica, void* buffer, uint32_t size);
void io_fs_write(pcb_t *pcb, char* path, char* data, uint32_t direccion_logica, uint32_t size);
void resize(pcb_t *pcb, uint32_t nuevo_tamano);
int solicitar_resize_memoria(uint16_t pid, uint32_t nuevo_tamano);

#endif