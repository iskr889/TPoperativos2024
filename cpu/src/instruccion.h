#ifndef I_CPU_
#define I_CPU_
#include "utils_cpu.h"


typedef enum {
    SUMA,
    RESTA,
    ASIGNACION
} operacion_t;


void actualizar_registro(cpu_reg_t* registros, registro_t registro, uint32_t valor, operacion_t operacion);
instruccion_t* fetch(pcb_t* pcb);
void decode(instruccion_t* instruccion, pcb_t* pcb);
void execute(instruccion_t* instruccion, pcb_t* pcb);
void* obtener_registro(cpu_reg_t* registros, registro_t registro);
extern int socket_memoria;


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