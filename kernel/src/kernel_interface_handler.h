#ifndef KERNELINTERFACEHANDLER_H_
#define KERNELINTERFACEHANDLER_H_

#include "main.h"

typedef struct {
    String nombre;
    int socket;
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
void manejar_interfaz(conexion_t handshake, int socket_interfaz);

String recibir_nombre(int socket);

#endif
