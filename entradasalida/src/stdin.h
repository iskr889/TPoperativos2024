#ifndef STDIN_H
#define STDIN_H

#include "main.h"

void interfaz_stdin (String nombre);

String leer_texto(int cant_caracteres);

bool stdin_procesar_instrucciones(int fd_kernel, int fd_memoria);

#endif
