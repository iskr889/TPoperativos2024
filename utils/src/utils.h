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
#define TIEMPO_UNIDAD_DE_TRABAJO(x) usleep(1000 * x)

typedef char* String;

typedef enum {
    CPU_CON_MEMORIA = 0,
    KERNEL_CON_MEMORIA,
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
} conexion_t; // Tipo de conexión especifico entre modulos

typedef enum {
    IO_GEN_SLEEP = 100,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
    IO_ERROR
} instruccionesIO_t; // Codigo de operación para cada instrucción de IO

typedef enum {
    MEMORY_PROCESS_CREATE = 200,    // Instrucción
    MEMORY_PROCESS_TERM,            // Instrucción
    MEMORY_PAGE_TABLE_ACCESS,       // Instrucción
    MEMORY_PROCESS_RESIZE,          // Instrucción
    MEMORY_USER_SPACE_ACCESS,       // Instrucción
    MEMORY_RESPONSE_OK,             // Respuesta OK general
    MEMORY_INVALID_FRAME,           // Devuelve en caso de querer leer un marco invalido
    MEMORY_INVALID_PID,             // Devuelve en caso de recibir un pid invalido
    MEMORY_INVALID_OPERATION,       // Devuelve en caso de recibir una operación invalidad (distinta de Write o Read)
    MEMORY_INVALID_READ,            // Devuelve en caso de fallar la lectura del User Space
    MEMORY_INVALID_WRITE,           // Devuelve en caso de fallar la escritura del User Space
    OUT_OF_MEMORY,                  // Devuelve en caso de quedarse sin memoria del User Space
    MEMORY_PID_PSEUDOCODE           // La CPU pide a la memoria un PC y la memoria envia el pseudocodigo correspondiente   
} instruccionesMemoria_t; // Codigo de operación para cada instrucción de Memoria

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
