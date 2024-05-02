#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/utils.h"
#include "../../utils/src/servidor.h"
#include "../../utils/src/cliente.h"

typedef struct {
    String   puerto_escucha;
    String   ip_memoria;
    String   puerto_memoria;
    String   ip_cpu;
    String   puerto_cpu_dispatch;
    String   puerto_cpu_interrupt;
    String   algoritmo_planificacion;
    uint16_t quantum;
    String*  recursos;
    String*  instancias_recursos;
    uint16_t grado_multiprogramacion;
} t_kernel_config;

/**
* @fn    Carga la configuración en la estructura kernel_config
* @brief Con el archivo config abierto solicita memoria y lo carga en la estructura kernel_config
*/
t_kernel_config* load_kernel_config(t_config* config);

/**
* @fn    Libera la memoria de la estructura kernel_config
* @brief Hace un free de la memoria solicitada incluido las variables de recursos e instancias_recursos
*/
void kernel_config_destroy(t_kernel_config* kernel_config);

/**
* @fn    Se conecta al modulo especificado
* @brief Crea una conexión con el modulo especificado por su puerto e ip. Además loguea el proceso.
*/
int conectarse_a_modulo(String modulo, String ip, String puerto, t_log* logger);

/**
* @fn    Queda escuchando por conexiones de otros modulos
* @brief Crea un servidor y escucha por si uno o mas modulos se conecten a él
*/
int modulo_escucha_conexiones_de(String otros_modulos, String puerto, t_log* logger);

/**
* @fn    Atiende las conexiones de otros modulos
* @brief En un hilo distinto atiende la conexion entrante sin bloquear el programa
*/
void atender_conexiones_al_modulo(pthread_t *hilo, int fd_servidor);

#endif