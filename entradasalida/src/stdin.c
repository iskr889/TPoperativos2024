#include "stdin.h"

extern t_interfaz_config* interfaz_config;
extern t_log *logger;
extern t_log *extra_logger;

void interfaz_stdin(String nombre)
{

    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, STDIN_CON_KERNEL, extra_logger);

    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, STDIN_CON_MEMORIA, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while (1){
        stdin_procesar_instrucciones(conexion_kernel, conexion_memoria);
    }

    close(conexion_kernel);
    close(conexion_memoria);
}

void io_stdin_read(int cant_caracteres, char **texto)
{

    while (1)
    {

        *texto = readline(">>Ingrese texto: ");

        if (texto == NULL)
        {
            log_info(extra_logger, "No se ingreso ningun texto");
        }
        else if (strlen(*texto) < cant_caracteres)
        {
            log_info(extra_logger, "Ingrese un texto mas largo, cantidad minima de caracteres: %i", cant_caracteres);
        }
        else
            break;
    }
}

void stdin_procesar_instrucciones(int fd_kernel, int fd_memoria)
{

    char *texto;
    paquete_t *paquete = recibir_paquete(fd_kernel);

    if (paquete == NULL)
        exit(EXIT_FAILURE);
    if (paquete->operacion != IO_STDIN_READ)
    {
        log_error(extra_logger, "Instruccion no valida!");
        return;
    }

    // formato: direccion, cant_caracteres

    // recibo direccion
    int direccion;

    payload_read(paquete->payload, &direccion, sizeof(int));
    if (direccion < 0)
    {
        log_error(extra_logger, "Direccion no valida");
        exit(EXIT_FAILURE);
    }

    // recibo cant_caracteres
    int cant_caracteres;

    payload_read(paquete->payload, &cant_caracteres, sizeof(int));
    if (cant_caracteres <= 0)
        {
            log_error(extra_logger, "Cantidad de caracteres recibida no valida, cant: %i", cant_caracteres);
        }

    // ejecuto la instruccion, pido el texto
    io_stdin_read(cant_caracteres, &texto);

    // **ENVIAR_PAQUETE** formato: direccion, cant_caracteres, texto
    int total_size = sizeof(int) + sizeof(int) + cant_caracteres;

    payload_t *buffer = payload_create(total_size);

    // direccion
    payload_add(buffer, &direccion, sizeof(int));

    // texto
    texto[cant_caracteres] = '\0';
    // printf(">> El texto ingresado es: %s\n", texto);
    payload_add_string(buffer, texto);

    paquete_t *paquete_a_enviar = crear_paquete(IO_STDIN_READ, buffer);

    if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK)
    {

        log_error(extra_logger, "No se pudo enviar el paquete a memoria");

        payload_destroy(buffer);
        liberar_paquete(paquete_a_enviar);
        liberar_paquete(paquete);

        return;
    }

    payload_destroy(buffer);
    liberar_paquete(paquete_a_enviar);
    liberar_paquete(paquete);

    free(texto);

    exit(EXIT_SUCCESS);
}

