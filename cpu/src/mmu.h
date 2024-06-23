#ifndef MMU_H
#define MMU_H
#include <stdint.h>
#include <math.h>
#include "main.h"
#include "tlb.h"

uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint16_t pid, int socket_memoria, uint32_t tam_pagina);
uint16_t estaActivaTLB();
uint32_t solicitarTamPagina(int socket_memoria);
uint32_t solicitarMarco(int socket_memoria, uint32_t pagina, uint16_t pid);
uint32_t recibirMarco(int socket_memoria);
uint32_t obtenerTamPagina(int socket_memoria);
uint32_t recibirTamPagina(int socket_memoria);
extern t_cpu_config cpu_config;
extern t_log*  logger;


typedef enum {

    TAM_PAGINA
} frames_cod_t;


#endif
