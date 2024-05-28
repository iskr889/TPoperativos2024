#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h> // printf()
#include <stdlib.h> // exit()
#include <string.h> // memset()
#include <unistd.h> // fork(), close()

#include <sys/socket.h> // socket(), accept(), connect()
#include <sys/types.h> // socket(), accept(), connect()
#include <netdb.h> // getaddrinfo()

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>

#include <readline/readline.h>
#include <assert.h>
#include <pthread.h> // pthread_create()

#define ERROR -1
#define OK     0

typedef char* String;

typedef enum {
        CPU_CON_MEMORIA,
     KERNEL_CON_MEMORIA,
    GENERIC_CON_MEMORIA,
      STDIN_CON_MEMORIA,
     STDOUT_CON_MEMORIA,
     DIALFS_CON_MEMORIA,
     KERNEL_CON_CPU_DISPATCH,
     KERNEL_CON_CPU_INTERRUPT,
    GENERIC_CON_KERNEL,
      STDIN_CON_KERNEL,
     STDOUT_CON_KERNEL,
     DIALFS_CON_KERNEL,
    HANDSHAKE_ERROR
} conexion_t ; // Tipo de conexión especifico entre modulos

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

/**
* @fn    Sale del programa imprimiendo un mensaje con el tipo de error
* @brief Sale del programa imprimiendo un mensaje con el tipo de error
*/
void error_exit(char *message);

#endif
