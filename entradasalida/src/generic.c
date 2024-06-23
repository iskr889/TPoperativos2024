#include "generic.h"

extern t_interfaz_config* interfaz_config;
extern t_log *logger;
extern t_log *extra_logger;

void interfaz_generica(String nombre) {

    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, GENERIC_CON_KERNEL, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while(1) {
        generic_procesar_instrucciones(conexion_kernel);
        enviar_operacion(OK, conexion_kernel);
    }
        

    close(conexion_kernel);
}

void generic_procesar_instrucciones(int socket) {
    paquete_t *paquete = recibir_paquete(socket);

    if(paquete == NULL)
        exit(EXIT_FAILURE);

    if(paquete->operacion != IO_GEN_SLEEP) {
        log_error(extra_logger, "Instruccion recibida por el kernel no reconocida!");
        return;
    }

    uint32_t tiempo_sleep;

    payload_read(paquete->payload, &tiempo_sleep, sizeof(uint32_t));

    payload_destroy(paquete->payload);
    liberar_paquete(paquete);

    io_gen_sleep(tiempo_sleep);
}

void io_gen_sleep(int unidades_trabajo) {
    if (unidades_trabajo <= 0) {
        log_error(extra_logger, "Unidades de trabajo invalidas!");
        exit(EXIT_FAILURE);
    }
    log_info(logger, "Operacion: IO_GEN_SLEEP %dms", unidades_trabajo);
    ESPERAR_X_MILISEGUNDOS(unidades_trabajo);
}
