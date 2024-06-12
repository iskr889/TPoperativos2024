void interfaz_stdin (String nombre){
    
    int conexion_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, STDIN_CON_KERNEL, extra_logger);
    
    int conexion_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, STDIN_CON_MEMORIA, extra_logger);

    enviar_nombre_interfaz(nombre, conexion_kernel);

    while(1)
        stdin_procesar_instrucciones(conexion_kernel, conexion_memoria);

    close(conexion_kernel);
    close(conexion_memoria);
}

void io_stdin_read(int cant_caracteres, char **texto){

    while(1){
        
        *texto = readline(">>Ingrese texto: ");

        if(texto == NULL){
            log_info(extra_logger, "No se ingreso ningun texto");
        }
        else if(strlen(*texto) < cant_caracteres){
            log_info(extra_logger, "Ingrese un texto mas largo, cantidad minima de caracteres: %i", cant_caracteres);
        }
        else
            break;
    }
}

void stdin_procesar_instrucciones(int fd_kernel, int fd_memoria){
    
    char *texto;
    paquete_t *paquete = recibir_paquete(fd_kernel);

    if(paquete == NULL)
        exit(EXIT_FAILURE);
    if(paquete->operacion != IO_STDIN_READ){
        log_error(extra_logger, "Instruccion no valida!");
        return;
    }

    // formato: cant_direcciones, direcciones, cant_caracteres
    
    // recibo cant direcciones
    int cant_direcciones;
    
    payload_read(paquete->payload, &cant_direcciones, sizeof(int));
    if(cant_direcciones <= 0){
        log_error(extra_logger, "Cantidad de direcciones invalida");
        exit(EXIT_FAILURE);
    }

    // recibo direcciones
    int *vector = (int *)malloc(cant_direcciones * sizeof(int));

    for(int i = 0; i < cant_direcciones; i ++){
        payload_read(paquete->payload, &vector[i], sizeof(int));
    }
    

    // recibo cantidad de caracteres 
    int cant_caracteres;
    payload_read(paquete->payload, &cant_caracteres, sizeof(int));
    if(cant_caracteres <= 0){
        log_error(extra_logger, "Cantidad de caracteres recibida no valida");
    }
    
    // ejecuto la instruccion, pido el texto
    io_stdin_read(cant_caracteres, &texto);

    // **ENVIAR_PAQUETE** formato: cant_direcciones, direcciones, texto
    int total_size = sizeof(int) + cant_direcciones * sizeof(int) + sizeof(int) + cant_caracteres;

    payload_t *buffer = payload_create(total_size);

    // cant_direcciones
    payload_add(buffer, &cant_direcciones, sizeof(int));

    // direcciones
    for(int i = 0; i < cant_direcciones; i++){
        payload_add(buffer, &vector[i], sizeof(int));
    }

    // texto
    texto[cant_caracteres] = '\0';
    //printf(">> El texto ingresado es: %s\n", texto);
    payload_add_string(buffer, texto);

    paquete_t *paquete_a_enviar = crear_paquete(IO_STDIN_READ, buffer);
    
    if(enviar_paquete(fd_memoria, paquete_a_enviar) != OK){
        
        log_error(extra_logger, "No se pudo enviar el paquete a memoria");
        
        payload_destroy(buffer);
        liberar_paquete(paquete_a_enviar);
        liberar_paquete(paquete);
        
        return;
    }

    payload_destroy(buffer);
    liberar_paquete(paquete_a_enviar);
    liberar_paquete(paquete);

    free(vector);
    free(texto);

    exit(EXIT_SUCCESS);
}

// ** MEMORIA ** 
void read_io_stdin(int fd_io){
    
    paquete_t *paquete = recibir_paquete(fd_io);

    if(paquete == NULL)
        exit(EXIT_FAILURE);
    if(paquete->operacion != IO_STDIN_READ){
        printf("Instruccion no valida!");
        return;
    }

    // formato: cant_direcciones, direcciones, texto(truncado)
    
    // recibo cant direcciones
    int cant_direcciones;
    
    payload_read(paquete->payload, &cant_direcciones, sizeof(int));
    if(cant_direcciones <= 0){
        printf("Cantidad de direcciones invalida");
        exit(EXIT_FAILURE);
    //DEBUG
    }else{
        printf(">> Cantidad de direcciones: %i\n", cant_direcciones);
    }

    // recibo direcciones
    int *vector = (int *)malloc(cant_direcciones * sizeof(int));

    for(int i = 0; i < cant_direcciones; i ++){
        payload_read(paquete->payload, &vector[i], sizeof(int));
        //DEBUG
        printf(">>Direccion numero[%i], contenido: %i\n", i, vector[i]);
    }

    // recibo texto
    //char *string = payload_read_string(paquete->payload);
    
    uint32_t length;
    payload_read(paquete->payload, &length, sizeof(length));
    printf(">>longitud de texto recibido: %i\n", length);
    String string = malloc(length);
    payload_read(paquete->payload, string, length);
    printf(">>texto recibido: %s\n", string);
}

// ** KERNEL **
void send_io_stdin_read(int socket) {

    int cant_caracteres = 12;
    int cant_direcciones = 4;
    int direcciones[] = {88, 56, 45, 22};

    int tam = sizeof(int) + cant_direcciones * sizeof(int) + sizeof(int);

    payload_t *payload = payload_create(tam);

    // cantidad de direcciones
    payload_add(payload, &cant_direcciones, sizeof(uint32_t));

    // direcciones
    for(int i = 0; i < cant_direcciones; i++) {
        payload_add(payload, &direcciones[i], sizeof(int));
    }

    // cantidad de caracteres
    payload_add(payload, &cant_caracteres, sizeof(int));

    paquete_t *paquete = crear_paquete(IO_STDIN_READ, payload);

    if(enviar_paquete(socket, paquete) != OK){
        printf("fallo al enviar el paquete a stdin");
        exit(EXIT_FAILURE);
    }

    payload_destroy(payload);
    liberar_paquete(paquete);
}