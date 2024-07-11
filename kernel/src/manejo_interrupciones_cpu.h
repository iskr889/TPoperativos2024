#ifndef MANEJOINTERRUPCIONESCPU_H_
#define MANEJOINTERRUPCIONESCPU_H_

#include "main.h"
#include "kernel_interface_handler.h"


void interrupt_handler();
void *manejo_interrupciones_cpu();
void actualizar_quantum(pcb_t *pcb, int quantum);
int ejecutar_IO(int socket_interfaz, uint16_t pid, char **instruccion_tokens);
bool verificar_instruccion(t_dictionary *diccionario, char **tokens);
char** split_string(char* str);
int obtener_tipo_instruccion(const char* tipo_str);
void agregar_instruccion(uint16_t pid, char** tokens);
void aumentar_grado_multiprogramacion();

#endif