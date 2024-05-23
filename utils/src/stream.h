#ifndef STREAM_H_
#define STREAM_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/bitarray.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <unistd.h>
#include <stdint.h>
#include <netdb.h>
#include <assert.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>



enum operacion_cpu {
    I_SET,
    I_MOV_IN,
    I_MOV_OUT,
    I_SUM,
    I_SUB,
    I_JNZ,
    I_RESIZE,
    I_COPY_STRING,
    I_WAIT,
    I_SIGNAL,
    I_IO_GEN_SLEEP,
    I_IO_STDIN_READ,
    I_IO_STDOUT_WRITE,
    I_IO_FS_CREATE,
    I_IO_FS_DELETE,
    I_IO_FS_TRUNCATE,
    I_IO_FS_WRITE,
    I_IO_FS_READ,
    I_EXIT,
};

typedef enum {
	MENSAJE,
    PCB,
    PAQUETE,
} op_code;



typedef enum operacion_cpu t_operacion_cpu;
typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code operacion;
	t_buffer* buffer;
} t_paquete;



typedef struct {
    int pid;
    char *msg;
} t_pcb;

void enviar_paquete(t_paquete* paquete, int socket_cliente);
t_list* recibir_paquete(int socket_cliente);
void enviar_mensaje(char* mensaje, int socket_cliente, uint8_t operacion);
void recibir_mensaje(int socket_cliente);
t_paquete* crear_paquete(op_code operacion);

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);

void eliminar_paquete(t_paquete* paquete);
void* serializar_paquete(t_paquete* , int);
void crear_buffer(t_paquete*);
void* recibir_buffer(int*, int);
int recibir_operacion(int socket);
void enviar_operacion(uint8_t operacion, int socket);
#endif



// Handshakes de los distintos servidores y clientes
// enum handshake 
// {
//     HANDSHAKE_consola,
//     HANDSHAKE_kernel,
//     HANDSHAKE_memoria,
//     HANDSHAKE_cpu,
//     HANDSHAKE_interrupt,
//     HANDSHAKE_dispatch,
//     HANDSHAKE_filesystem,
//     HANDSHAKE_ok_continue
// };
// typedef enum handshake t_handshake;

// Prototipos
