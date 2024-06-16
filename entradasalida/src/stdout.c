#include "stdout.h"

extern t_interfaz_config* interfaz_config;
extern t_log *logger;
extern t_log *extra_logger;


void interfaz_stdout(String nombre)
{

    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, STDOUT_CON_KERNEL, extra_logger);

    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, STDOUT_CON_MEMORIA, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while (1){
        stdout_procesar_instrucciones(conexion_kernel, conexion_memoria);
    }
}

void io_stdout_write(int direccion, int cant_caracteres, int fd_memoria)
{
    // ** ENVIAR LA INFO**
    int tam = sizeof(int) + sizeof(int);

    payload_t *payload = payload_create(tam);

    // direccion
    payload_add(payload, &direccion, sizeof(uint32_t));

    // cantidad de caracteres
    payload_add(payload, &cant_caracteres, sizeof(int));

    paquete_t *paquete_a_enviar = crear_paquete(IO_STDOUT_WRITE, payload);

    if(enviar_paquete(fd_memoria, paquete_a_enviar) != OK){
        printf("fallo al enviar el paquete a stdin");
        exit(EXIT_FAILURE);
    }

    // ** RECIBO LA INFO**
    paquete_t *paquete_a_recibir = recibir_paquete(fd_memoria);

    if (paquete_a_recibir == NULL)
        exit(EXIT_FAILURE);
    if (paquete_a_recibir->operacion != IO_STDOUT_WRITE)
    {
        log_error(extra_logger, "Instruccion no valida!");
        return;
    }

    char *texto = payload_read_string(paquete_a_recibir->payload);

    // ** MUESTRO LA INFO**
    log_info(logger, ">>Texto recibido: %s", texto);

    liberar_paquete(paquete_a_recibir);
    payload_destroy(payload);
    liberar_paquete(paquete_a_enviar);
}

void stdout_procesar_instrucciones(int fd_kernel, int fd_memoria)
{

    paquete_t *paquete = recibir_paquete(fd_kernel);

    // lo que recibo de kernel es una direccion de memoria y una cantidad en adelante

    if (paquete == NULL)
        exit(EXIT_FAILURE);
    if (paquete->operacion != IO_STDOUT_WRITE)
    {
        log_error(extra_logger, "Instruccion no valida!");
        return;
    }

    // recibo la primer direccion
    int puntero;

    payload_read(paquete->payload, &puntero, sizeof(int));
    if (puntero < 0)
    {
        log_error(extra_logger, "Direccion Invalida");
        exit(EXIT_FAILURE);
    }

    // recibo cantidad de caracteres
    int cant_caracteres;
    payload_read(paquete->payload, &cant_caracteres, sizeof(int));
    if (cant_caracteres <= 0)
    {
        log_error(extra_logger, "Cantidad de caracteres recibida no valida");
        exit(EXIT_FAILURE);
    }
    
    // DEBUG
    log_info(extra_logger, "direccion: %i, cant_caracteres: %i", puntero, cant_caracteres);

    // ejecuto la instruccion
    io_stdout_write(puntero, cant_caracteres, fd_memoria);

    close(fd_memoria);
    close(fd_kernel);
    liberar_paquete(paquete);

    exit(EXIT_SUCCESS);
}