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
#include <commons/temporal.h>

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
    MEMORY_PROCESS_CREATE = 200,
    MEMORY_PROCESS_TERM,
    MEMORY_PAGE_TABLE_ACCESS,
    MEMORY_PROCESS_RESIZE,
    MEMORY_USER_SPACE_ACCESS,
    OUT_OF_MEMORY
} instruccionesMemoria_t; // Codigo de operación para cada instrucción de Memoria

//A partir de aca copie lo que estaba en el codigo de instrucciones. Hay que revisar.
typedef enum {
    I_SET,
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
    AX,
    BX,
    CX,
    DX,
    EAX,
    EBX,
    ECX,
    EDX,
    PC
} registro_t;

typedef struct {
    tipo_instruccion_t tipo;
    registro_t registro1;
    registro_t registro2;
    uint32_t valor;
    char *interfaz;
    char *recurso;
    char *nombreArchivo;
} instruccion_t;

/*
typedef struct {
    uint32_t pc;
    uint8_t ax;
    uint8_t bx;
    uint8_t cx;
    uint8_t dx;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t si;
    uint32_t di;
} cpu_reg_t;*/


typedef enum{
    FINALIZADO=1,
    IO,
    DESALOJO_QUANTUM,
    WAIT,
    SIGNAL,
}interrupciones_t;


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
