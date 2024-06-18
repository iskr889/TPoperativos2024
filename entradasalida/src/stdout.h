#ifndef STDOUT_H
#define STDOUT_H

#include "main.h"

void interfaz_stdout(String nombre);

bool stdout_procesar_instrucciones(int fd_kernel, int fd_memoria);

#endif
