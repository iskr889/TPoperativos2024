#ifndef CLIENTE_H_
#define CLIENTE_H_

#include "utils.h"

/**
* @fn    Crea una conexion con el servidor
* @brief El cliente crea una conexion con el servidor en la ip y puerto especificado
*/
int crear_conexion(String ip, String puerto);

/**
* @fn    Handshake del cliente al servidor
* @brief Inicia un Handshake entre el servidor y el cliente
*/
int handshake_con_servidor(int socket_servidor, modulos_t handshake);

/**
* @fn    Se conecta al modulo especificado
* @brief Crea una conexión con el modulo especificado por su puerto e ip. Además loguea el proceso.
*/
int conectarse_a_modulo(String modulo, String ip, String puerto, modulos_t tipo, t_log* logger);

#endif
