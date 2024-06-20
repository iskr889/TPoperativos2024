#ifndef MANEJOINTERRUPCIONESCPU_H_
#define MANEJOINTERRUPCIONESCPU_H_


#include "main.h"

void manejador_de_interrupciones();
void *manejo_interrupciones_cpu();
void actualizar_quantum(pcb_t *pcb, int quantum);
//void *thread_hilo_mock_IO(void *arg);
int ejecutar_IO(char **instruccion_tokens);
bool verificar_instruccion(t_dictionary *diccionario, char **tokens);
char** split_string(char* str);
int obtener_tipo_instruccion(const char* tipo_str);

#endif