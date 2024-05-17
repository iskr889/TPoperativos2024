#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "utils.h"

/**
* @fn    Inicia una conexión en un puerto dado
* @brief Crea un socket y queda escuchando el puerto indicado
*/
int iniciar_servidor(String puerto);

/**
* @fn    El servidor acepta a los clientes
* @brief Forkea el programa cada vez que acepta un cliente y continua escuchando por mas conexiones [BLOQUEANTE]
*/
int aceptar_clientes(int socket_servidor);

/**
* @fn    Handshake del servidor al cliente
* @brief Inicia un Handshake entre el cliente y el servidor
*/
int handshake_con_cliente(int socket_cliente);

/**
* @fn    Nuevo hilo para efectuar el handshake con el cliente
* @brief Se ejecuta un hilo de ejecución para efectuar el handshake con el cliente y que el servidor pueda continuar la atención a otros clientes
*/
void* thread_handshake_con_cliente(void* fd_cliente);

/**
* @fn    Nuevo hilo para aceptar cliente
* @brief Se ejecuta un hilo de ejecución para aceptar clientes y que el servidor pueda continuar su ejecución sin ser bloqueado
*/
void *thread_aceptar_clientes(void *arg);

/**
* @fn    Queda escuchando por conexiones de otros modulos
* @brief Crea un servidor y escucha por si uno o mas modulos se conecten a él
*/
int modulo_escucha_conexiones_de(String nombre_modulos, String puerto, t_log* logger);

/**
* @fn    Atiende las conexiones de otros modulos
* @brief En un hilo distinto atiende la conexion entrante sin bloquear el programa
*/
void atender_conexiones_al_modulo(pthread_t *hilo, int fd_servidor);

/**
* @fn    Maneja los comandos del modulo conectado
* @brief Luego del handshake el thread queda esperando por comandos al servidor
*/
void manejar_comandos(handshake_t handshake, int socket_cliente);

#endif
