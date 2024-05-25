#ifndef MAIN_H
#define MAIN_H

#include "../../utils/src/cliente.h"
#include "../../utils/src/utils.h"

t_log *logger;

typedef struct{
    int tamanio_instruccion;
    char *instruccion;
    int u_trabajo;
}t_instruccion_generica;
typedef struct
{
    String tipo_interfaz;
    uint64_t tiempo_u_trabajo;
    String ip_kernel;
    String puerto_kernel;
    String ip_memoria;
    String puerto_memoria;
    String path_base_dialfs;
    uint64_t block_size;
    uint64_t block_count;
} t_interfaz_config;


// /**
// * @fn    Carga la configuración en la estructura interfaz_config
// * @brief Con el archivo config abierto solicita memoria y lo carga en la estructura interfaz_config
// */
// t_interfaz_config* load_interfaz_config(t_config* config);

/*** CONFIG INTERFAZ ***/
t_interfaz_config *load_generic_config(t_config *config);

/*** INSTRUCCIONES **/
void io_gen_sleep(int unidades_trabajo);

/*** FUNCIONES AUXILIARES ***/
t_interfaz_config *crear_interfaz_config();
char *obtener_interfaz(char *ruta);
t_instruccion_generica *recibir_instruccion(int fd);

/*** INTERFACES **/
void interfaz_generica(char *nombre, char *ruta);
// terminar
void interfaz_stdin(char *nombre, char *ruta);
void interfaz_stdout(char *nombre, char *ruta);
void interfaz_dialFS(char *nombre, char *ruta);
#endif