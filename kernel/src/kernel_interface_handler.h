#ifndef KERNELINTERFACEHANDLER_H_
#define KERNELINTERFACEHANDLER_H_

#include "main.h"

typedef enum {
    GENERIC,
    STDIN,
    STDOUT,
    DIALFS
} tipo_io_t;

typedef struct {
    int socket;
    tipo_io_t tipo;
} interfaz_t;

/**
* @fn    Maneja las conexiones de las interfaces
* @brief En un hilo distinto atiende multiples conexiones de interfaces sin bloquear el programa
*/
void manejador_de_interfaces(int fd_servidor);

/**
* @fn    Nuevo hilo para aceptar interfaz
* @brief Se ejecuta un hilo de ejecución para aceptar interfaces y que el kernel pueda continuar su ejecución sin ser bloqueado
*/
void *thread_aceptar_interfaces(void *arg);

/**
* @fn    El kernel acepta interfaces
* @brief Acepta interfaces en un hilo de ejecución a parte
*/
int aceptar_interfaces(int socket_servidor);

/**
* @fn    Nuevo hilo para efectuar el handshake con la interfaz
* @brief Se ejecuta un hilo de ejecución para efectuar el handshake con la interfaz y que el kernel pueda continuar la atención a otras interfaces
*/
void* thread_handshake_con_interfaz(void* fd_interfaz);

/**
* @fn    Handshake del kernel a la interfaz
* @brief Inicia un Handshake entre la interfaz y el kernel
*/
int handshake_con_interfaz(int socket_interfaz);

/**
* @fn    Maneja la interfaz conectada
* @brief Maneja la interfaz conectada según el tipo
*/
int manejar_interfaz(conexion_t handshake, int socket_interfaz);

/**
* @fn    Espera recibir el nombre de la interfaz conectada
* @brief Recibe el nombre de la interfaz conectada
*/
String recibir_nombre_interfaz(int socket);

/**
* @fn    Envia IO_GEN_SLEEP
* @brief Envia la instrucción IO_GEN_SLEEP a la interfaz generica conectada
*/
void send_io_gen_sleep(int socket, uint32_t tiempo);

/**
* @fn    Envia IO_STDIN_READ
* @brief Envia la instrucción IO_STDIN_READ a la interfaz stdin conectada
*/
void send_io_stdin_read(int socket, uint32_t direccion, uint32_t cant_caracteres);

/**
 * @fn Envia IO_STDOUT_WRITE
 * @brief Envia la instruccion IO_STDOUT_WRITE a la interfas stdout conectada 
*/
void send_io_stdout_write(int fd_io, uint32_t direccion, uint32_t cant_caracteres);

#endif
