#ifndef STDOUT_H
#define STDOUT_H

#include "main.h"

void interfaz_stdout(String nombre);

void io_stdout_write(int puntero, int cant_caracters, int fd_memoria);

void stdout_procesar_instrucciones(int fd_kernel, int fd_memoria);


#endif
