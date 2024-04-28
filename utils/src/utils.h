#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>

#define ERROR -1
#define OK     0

typedef char* String;

/**
* @fn    Inicia un logger
* @brief Crea un logger y devuelve un puntero a t_log para ser utilizado
*/
t_log* iniciar_logger(String path, String name, bool is_active_console, t_log_level level);

/**
* @fn    Inicia un config
* @brief Crea un config y devuelve un puntero a t_config para ser utilizado
*/
t_config* iniciar_config(String path);


#endif
