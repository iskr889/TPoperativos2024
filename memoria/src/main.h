#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include <utils/hello.h>

t_log* iniciar_logger(void);
t_config* iniciar_config(void);
void error_exit(char *);


void crear_proceso(void);
void terminar_proceso(void);
void acceso_tabla_paginas(void);
void ajustar_lenght_proceso(void);
void acceso_espacio_usuario(void);
#endif