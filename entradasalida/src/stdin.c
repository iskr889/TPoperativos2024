#include "../../utils/src/cliente.h"
#include "../../utils/src/utils.h"
#include "../../utils/src/serializacion.h"
#include "main.h"

void interfaz_stdin (String nombre){
    
    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, STDIN_CON_KERNEL, extra_logger);
    
    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, STDIN_CON_MEMORIA, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while(1)
        stdin_procesar_instrucciones(conexion_kernel, conexion_memoria);

    close(conexion_kernel);
}

void io_stdin_read(){
    
    // leo el texto
    char *texto = readline(">>Ingrese texto: ");

    if(texto == NULL){
        log_info(extra_logger, "No se ingreso ningun texto");
        return;
    }

    // agrego el texto, falta agregar las direcciones
    paquete_t *payload = payload_create(strlen(texto));
    payload_add(payload, texto, strlen(texto));
    paquete_t *paquete = crear_paquete(IO_STDIN_READ, payload);
    
    // lo envio a memoria
    if(enviar_paquete(fd_memoria, paquete) != OK)
        exit(EXIT_FAILURE);
    
    payload_destroy(payload);
    liberar_paquete(paquete);

}

void stdin_procesar_instrucciones(int fd_kernel, int fd_memoria){
    
    paquete_t *paquete = recibir_paquete(fd_kernel);

    if(paquete == NULL)
        exit(EXIT_FAILURE);
    if(paquete->operacion != IO_STDIN_READ){
        log_error(extra_logger, "Instruccion no valida!");
        return;
    }

    // recibe de kernel direccines de memoria fisica donde guardar las cosas
    char *direccion;
    
    // puede ser una o mas direcciones
    payload_read(paquete->payload);


    // ejecuto la instruccion
    io_strin_read(direccion);
}