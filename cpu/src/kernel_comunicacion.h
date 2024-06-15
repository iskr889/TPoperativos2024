#ifndef KERNEL_COMUNICACION_H_
#define KERNEL_COMUNICACION_H_

#include "main.h"

void enviar_io_gen_sleep(int socket, uint32_t tiempo);
uint32_t recibir_io_gen_sleep(int socket);

#endif