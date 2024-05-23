#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "utils.h"

/**
* @fn    Queda escuchando por conexiones de otros modulos
* @brief Crea un servidor y escucha el puerto indicado por conexiones entrantes
*/
int escuchar_conexiones_de(String nombre_modulos, String puerto, t_log* logger);

/**
* @fn    Inicia una servidor en el puerto indicado
* @brief Crea un socket y queda escuchando el puerto indicado (utiliza su propia ip)
*/
int iniciar_servidor(String puerto);

/**
* @fn    Espera conexión de un modulo
* @brief Espera la conexión del modulo especificado por "conexion_t tipo_de_conexion" al servidor
*/
int esperar_conexion_de(conexion_t tipo_de_conexion, int socket_servidor);

/**
* @fn    Handshake del servidor al cliente
* @brief Inicia un Handshake entre el cliente y el servidor
*/
conexion_t handshake_con_cliente(int socket_cliente);

#endif
