#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h> // printf()
#include <stdlib.h> // exit()
#include <string.h> // memset()
#include <unistd.h> // fork(), close()
#include <stdint.h>

#include <sys/socket.h> // socket(), accept(), connect()
#include <sys/types.h> // socket(), accept(), connect()
#include <netdb.h> // getaddrinfo()

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/temporal.h>

#include <readline/readline.h>
#include <assert.h>
#include <pthread.h> // pthread_create()

#define ERROR -1
#define OK     0
#define ESPERAR_X_MILISEGUNDOS(x) usleep(1000 * x)

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
    MEMORY_INVALID_PATH,            // Devuelve en caso que el path de pseudocodigo recibido del kernel sea invalido
    OUT_OF_MEMORY,                  // Devuelve en caso de quedarse sin memoria del User Space
    MEMORY_PID_PSEUDOCODE,          // La CPU pide a la memoria un PC y la memoria envia el pseudocodigo correspondiente
    MEMORY_PAGE_SIZE,               // Envia el tamaño de pagina a la cpu
} instruccionesMemoria_t; // Codigo de operación para cada instrucción de Memoria

typedef enum {
    I_SET = 300,
    I_SUM,
    I_SUB,
    I_JNZ,
    I_IO_GEN_SLEEP,
    I_MOV_IN,
    I_MOV_OUT,
    I_RESIZE,
    I_COPY_STRING,
    I_WAIT,
    I_SIGNAL,
    I_IO_STDIN_READ,
    I_IO_STDOUT_WRITE,
    I_IO_FS_CREATE,
    I_IO_FS_DELETE,
    I_IO_FS_TRUNCATE,
    I_IO_FS_WRITE,
    I_IO_FS_READ,
    I_EXIT,
} tipo_instruccion_t;

typedef enum {
    AX = 400,
    BX,
    CX,
    DX,
    EAX,
    EBX,
    ECX,
    EDX,
    PC,
    SI,
    DI
} registro_t;

typedef enum {
    FINALIZADO = 500,
    IO,
    DESALOJO_QUANTUM,
    WAIT,
    SIGNAL,
    INTERRUPTED_BY_USER,
} interrupciones_t;

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
