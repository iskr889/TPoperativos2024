void interfaz_stdin (String nombre){
    
    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, STDIN_CON_KERNEL, extra_logger);
    
    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, STDIN_CON_MEMORIA, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while(1)
        stdin_procesar_instrucciones(conexion_kernel, conexion_memoria);

    close(conexion_kernel);
}

char *leer_texto(char *texto){
    

    return texto;
}

void io_stdin_read(int fd_memoria, int *direcciones, int cant_caracteres{
    
    // leo el texto
    char *texto;

    while(1){
        texto = readline(">>Ingrese texto: ");

        if(texto == NULL){
            log_info(extra_logger, "No se ingreso ningun texto");
        }
        if else(strlen(texto) < cant_caracteres){
            log_info(extra_logger, "Ingrese un texto mas largo, cantidad minima  de caracteres: %i", cant_caracteres);
        }
        else
            break;
    }
    
    // primero las direcciones y luego el texto
    
    // tamaño = longitud + direcciones + texto

    size_t longitud = sizeof(vector) / sizeof(vector[0]);
    
    int tamanio = sizeof(int) + longitud * sizeof(int) + cant_caracteres;
    
    payload_t *payload = payload_create(tamanio);
    
    payload_add(payload, &longitud, sizeof(int));

    for(int i = 0; i < longitud; i++){
        payload_add(payload, &direcciones[i], sizeof(int));
    }
    
    payload_add(payload, texto, cant_caracteres);
    
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

    // recibe de kernel una o varias direcciones
    int cant_direcciones;
    
    // puede ser una o mas direcciones
    payload_read(paquete->payload, &cant_direcciones, sizeof(int));
    if(cant_direcciones <= 0){
        log_error(extra_logger, "Cantidad de direcciones invalida");
        exit(EXIT_FAILURE);
    }

    // creo el vector
    int *vector = (int *)malloc(cant_direcciones * sizeof(int));

    for(int i = 0; i < cant_direcciones; i ++){
        payload_read(paquete->payload, &vector[i], sizeof(int));
    }

    // cantidad de caracteres 
    int cant_caracteres;
    payload_read(paquete->payload, &cant_caracteres, sizeof(int));
    if(cant_caracteres <= 0){
        log_error(extra_logger, "Cantidad de caracteres recibida no valida");
    }
    // ejecuto la instruccion
    io_stdin_read(fd_memoria, vector, cant_caracteres);

    free(vector);
}