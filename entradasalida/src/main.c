#include "main.h"

t_log *logger;

int main(int argc, char *argv[])
{
    
    if(argc != 3){
        log_warning(logger, "Ingrese correctamente los argumentos");
        return -1;
    }
    // char *ruta = argv[2];
    // char *nombre = argv[1];

    logger = iniciar_logger("entradaSalida.log",argv[1], 1, LOG_LEVEL_INFO);

    char *t_interfaz = obtener_interfaz(argv[2]);

    if (strcmp(t_interfaz, "GENERIC") == 0)
    {
        interfaz_generica(argv[2]);
    }
    else if (strcmp(t_interfaz, "STDIN") == 0)
    {
        interfaz_stdin(argv[2]);
    }
    else if (strcmp(t_interfaz, "STDOUT") == 0)
    {
        interfaz_stdout(argv[2]);
    }
    else if (strcmp(t_interfaz, "DialFS") == 0)
    {
        interfaz_dialFS(argv[2]);
    }
    else
    {
        log_error(logger, "Tipo de interfaz incorrecto!");
        return -1;
    }

    log_destroy(logger);

    return OK;
}

char *obtener_interfaz(char *ruta)
{

    // creo la interfaz
    t_config *config = iniciar_config(ruta);

    char *interfaz = config_get_string_value(config, "TIPO_INTERFAZ");

    char *t_interfaz = string_duplicate(interfaz);

    config_destroy(config);

    return t_interfaz;
}

t_interfaz_config *crear_interfaz_config()
{

    t_interfaz_config *interfaz_config = malloc(sizeof(t_interfaz_config));

    if (interfaz_config == NULL)
    {
        perror("Fallo malloc");
        exit(EXIT_FAILURE);
    }

    return interfaz_config;
}

t_interfaz_config *load_generic_config(t_config *config)
{

    t_interfaz_config *interfaz_generica = crear_interfaz_config();

    interfaz_generica->tipo_interfaz = config_get_string_value(config, "TIPO_INTERFAZ");
    interfaz_generica->tiempo_u_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    interfaz_generica->ip_kernel = config_get_string_value(config, "IP_KERNEL");
    interfaz_generica->puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

    return interfaz_generica;
}

void io_gen_sleep(int unidades_trabajo)
{
    if (unidades_trabajo < 0)
    {
        log_error(logger, "Unidades de trabajo Negativas!");
        exit(EXIT_FAILURE);
    }
    sleep(unidades_trabajo);
}

t_instruccion_generica *recibir_instruccion(int socket_cliente)
{
    t_instruccion_generica *instrucciones = malloc(sizeof(t_instruccion_generica));
    int tamanio;
    int u_trabajo;
    void *buffer;

    recv(socket_cliente, &tamanio, sizeof(int), MSG_WAITALL);
    instrucciones->instruccion = malloc(tamanio + 1);
    buffer = malloc(tamanio);
    recv(socket_cliente, buffer, tamanio, MSG_WAITALL);
    memcpy(instrucciones->instruccion, buffer, tamanio);
    instrucciones->instruccion[tamanio] = '\0';
    free(buffer);
    recv(socket_cliente, &u_trabajo, sizeof(int), MSG_WAITALL);
    instrucciones->u_trabajo = u_trabajo;
    return instrucciones;
}

void interfaz_generica(char *ruta)
{

    t_config *config = iniciar_config(ruta);

    t_interfaz_config *generic_config = load_generic_config(config);

    int conexion_kernel = conectarse_a_modulo("KERNEL", generic_config->ip_kernel, generic_config->puerto_kernel, GENERIC_CON_KERNEL, logger);

    while (1)
    {
        log_info(logger, ">>Esperando instruccin....");

        t_instruccion_generica *instruccion = recibir_instruccion(conexion_kernel);

        if (strcmp(instruccion->instruccion, "IO_GEN_SLEEP") == 0)
        {

            if (instruccion->u_trabajo <= -1)
            {
                log_error(logger, "No fue posible obtener las unidades de trabajo");
                exit(EXIT_FAILURE);
            }
            else if (instruccion->u_trabajo == 0)
            {
                log_info(logger, "Cerrando interfaz.");

                // libero
                close(conexion_kernel);
                config_destroy(config);
                free(generic_config);

                exit(EXIT_SUCCESS);
            }

            io_gen_sleep(instruccion->u_trabajo);
            log_info(logger, "Instruccion completada con exito!");
        }
        else
        {
            log_error(logger, "La instruccion recibida no es valida!");

            exit(EXIT_FAILURE);
        }
    }
}

void interfaz_stdin(char *ruta) {}
void interfaz_stdout(char *ruta) {}
void interfaz_dialFS(char *ruta) {}
