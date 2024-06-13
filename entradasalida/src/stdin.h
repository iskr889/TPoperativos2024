#ifndef STDIN_H
#define STDIN_H

#include "main.h"

void interfaz_stdin (String nombre);

void io_stdin_read(int cant_caracteres, char **texto);

void stdin_procesar_instrucciones(int fd_kernel, int fd_memoria);

void read_io_stdin(int fd_io);

void send_io_stdin_read(int socket);

#endif
