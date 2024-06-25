#include "manejar_instrucciones.h"

extern t_log* logger;
extern t_log* extra_logger;
extern t_dictionary *procesos;
extern t_memoria_config *memoria_config;

extern int conexion_kernel, conexion_cpu;

static Proceso_t *proceso_get(uint16_t pid);

void manejar_instrucciones_kernel() {
    pthread_t hilo_instrucciones_kernel;

    if (pthread_create(&hilo_instrucciones_kernel, NULL, thread_instrucciones_kernel, NULL) != 0) {
        perror("No se pudo crear el hilo de instrucciones kernel");
        exit(EXIT_FAILURE);
    }

    pthread_join(hilo_instrucciones_kernel, NULL); // Para que no cierre el main()
}

void *thread_instrucciones_kernel(void *arg) {

    while(1) {

        paquete_t *paquete = recibir_paquete(conexion_kernel);

        if(paquete == NULL) {
            fprintf(stderr, "Error al recibir paquete del Kernel\n");
            pthread_exit(0);
        }

        ESPERAR_X_MILISEGUNDOS(memoria_config->retardo_respuesta);

        switch (paquete->operacion) {
            case MEMORY_PROCESS_CREATE:
                instruccion_process_create(paquete->payload);
                break;
            case MEMORY_PROCESS_TERM:
                instruccion_process_terminate(paquete->payload);
                break;
            default:
                log_error(extra_logger, "Error al tratar de identificar el codigo de operación del Kernel");
                break;
        }

        payload_destroy(paquete->payload);
        liberar_paquete(paquete);
    }

    pthread_exit(0);
}

void manejar_instrucciones_cpu() {
    pthread_t hilo_instrucciones_cpu;

    if (pthread_create(&hilo_instrucciones_cpu, NULL, thread_instrucciones_cpu, NULL) != 0) {
        perror("No se pudo crear el hilo de instrucciones cpu");
        exit(EXIT_FAILURE);
    }

    pthread_detach(hilo_instrucciones_cpu);
}

void *thread_instrucciones_cpu(void *arg) {

    while(1) {

        paquete_t *paquete = recibir_paquete(conexion_cpu);

        if(paquete == NULL) {
            fprintf(stderr, "Error al recibir paquete de la CPU\n");
            pthread_exit(0);
        }

        ESPERAR_X_MILISEGUNDOS(memoria_config->retardo_respuesta);

        switch (paquete->operacion) {
            case MEMORY_PID_PSEUDOCODE:
                instruccion_enviar_pseudocodigo(paquete->payload);
                break;
            case MEMORY_PAGE_TABLE_ACCESS:
                instruccion_pageTable_access(paquete->payload);
                break;
            case MEMORY_PROCESS_RESIZE:
                instruccion_process_resize(paquete->payload);
                break;
            case MEMORY_USER_SPACE_ACCESS:
                instruccion_userspace_access(paquete->payload, conexion_cpu);
                break;
            default:
                log_error(extra_logger, "Error al tratar de identificar el codigo de operación de la CPU");
                break;
        }

        payload_destroy(paquete->payload);
        liberar_paquete(paquete);
    }

    pthread_exit(0);
}

void instruccion_process_create(payload_t* payload) {

    char full_path[BUFF_SIZE];

    if (getcwd(full_path, sizeof(full_path)) == NULL) {
        perror("Error en getcwd()");
        exit(EXIT_FAILURE);
    }

    uint16_t pid;

    payload_read(payload, &pid, sizeof(uint16_t));

    String path = payload_read_string(payload);

    strcat(full_path, path); // Concateno en el directorio actual de trabajo la carpeta con el pseudocodigo

    free(path);

    t_list *instrucciones = leer_pseudocodigo(full_path);

    if(instrucciones == NULL) {
        puts("Path invalido!");
        return;
    }

    crear_proceso(pid, instrucciones);

    log_debug(extra_logger, "Proceso [PID: %d] creado", pid);
    log_debug(extra_logger, "Pseudocodigo en: %s", full_path);

    // imprimir_instrucciones(instrucciones);
}

void instruccion_process_terminate(payload_t* payload) {

    uint16_t pid;

    payload_read(payload, &pid, sizeof(uint16_t));

    liberar_proceso(pid);

    log_debug(extra_logger, "Proceso [PID: %d] finalizado", pid);
}

void instruccion_pageTable_access(payload_t* payload) {

    uint16_t pid;
    uint32_t numero_pagina;

    payload_read(payload, &pid, sizeof(uint16_t));
    payload_read(payload, &numero_pagina, sizeof(uint32_t));

    Proceso_t* proceso = proceso_get(pid);

    if (proceso == NULL) {
        payload_t *payload = payload_create(0);
        paquete_t *respuesta = crear_paquete(MEMORY_INVALID_PID, payload);
        enviar_paquete(conexion_cpu, respuesta);
        payload_destroy(payload);
        liberar_paquete(respuesta);
        log_error(extra_logger, "No existe en la memoria el proceso con [PID: %d] solicitado por la CPU", pid);
        return;
    }
    
    int marco = acceder_marco(proceso, numero_pagina);

    payload_t *payload_marco = payload_create(sizeof(int));
    payload_add(payload_marco, &marco, sizeof(int));

    int cod_op = marco < 0 ? MEMORY_INVALID_FRAME : MEMORY_RESPONSE_OK;

    paquete_t *respuesta = crear_paquete(cod_op, payload_marco);
    enviar_paquete(conexion_cpu, respuesta);

    payload_destroy(payload_marco);
    liberar_paquete(respuesta);

    log_debug(extra_logger, "Proceso [PID: %d] enviando marco %d correspondiente a la pagina %d", pid, marco, numero_pagina);
}

void instruccion_process_resize(payload_t* payload) {

    uint16_t pid;
    uint32_t new_size;

    payload_read(payload, &pid, sizeof(uint16_t));
    payload_read(payload, &new_size, sizeof(uint32_t));

    Proceso_t* proceso = proceso_get(pid);

    if (proceso == NULL) {
        payload_t *payload = payload_create(0);
        paquete_t *respuesta = crear_paquete(MEMORY_INVALID_PID, payload);
        enviar_paquete(conexion_cpu, respuesta);
        payload_destroy(payload);
        liberar_paquete(respuesta);
        log_error(extra_logger, "No existe en la memoria el proceso con [PID: %d] solicitado por la CPU", pid);
        return;
    }

    int cod_op = resize_proceso(proceso, new_size) ? MEMORY_RESPONSE_OK : OUT_OF_MEMORY;
    paquete_t *respuesta = crear_paquete(cod_op, NULL);
    enviar_paquete(conexion_cpu, respuesta); // Envia paquete vacio con cod_op OK o Out of memory
    liberar_paquete(respuesta);

    log_debug(extra_logger, "Proceso [PID: %d] redimensionado a %d paginas", pid, new_size);
}

void instruccion_userspace_access(payload_t* payload, int fd_conexion) {

    char operacion;
    uint32_t address;
    uint32_t size;

    payload_read(payload, &operacion, sizeof(char));
    payload_read(payload, &address, sizeof(uint32_t));
    payload_read(payload, &size, sizeof(uint32_t));

    payload_t *payload_respuesta;

    char *buffer_data = malloc(size);

    int cod_op;

    if (operacion == 'R') {
        
        bool resultado = leer_memoria(address, buffer_data, size);

        cod_op = resultado ? MEMORY_RESPONSE_OK : MEMORY_INVALID_READ;

        payload_respuesta = payload_create(size);

        payload_add(payload_respuesta, buffer_data, size);

    } else if (operacion == 'W') {

        payload_read(payload, buffer_data, size);

        bool resultado = escribir_memoria(address, buffer_data, size);

        cod_op = resultado ? MEMORY_RESPONSE_OK : MEMORY_INVALID_WRITE;

        char ok[] = "OK";

        payload_respuesta = payload_create(sizeof(uint32_t) + strlen(ok) + 1);

        payload_add_string(payload_respuesta, ok);
        
    } else {
        cod_op = MEMORY_INVALID_OPERATION;
    }

    paquete_t *respuesta = crear_paquete(cod_op, payload_respuesta);
    enviar_paquete(fd_conexion, respuesta);

    payload_destroy(payload_respuesta);
    liberar_paquete(respuesta);
    free(buffer_data);

    log_debug(extra_logger, "Accediendo %d posiciones de memoria desde la dirección %d", size, address);
}

void instruccion_enviar_pseudocodigo(payload_t* payload) {

    uint16_t pid;
    uint32_t programCounter;

    payload_read(payload, &pid, sizeof(uint16_t));
    payload_read(payload, &programCounter, sizeof(uint32_t));

    Proceso_t* proceso = proceso_get(pid);

    if (proceso == NULL) {
        payload_t *payload = payload_create(0);
        paquete_t *respuesta = crear_paquete(MEMORY_INVALID_PID, payload);
        enviar_paquete(conexion_cpu, respuesta);
        payload_destroy(payload);
        liberar_paquete(respuesta);
        log_error(extra_logger, "No existe en la memoria el proceso con [PID: %d] solicitado por la CPU", pid);
        return;
    }

    String instruccion = list_get(proceso->instrucciones, programCounter);

    payload_t *payload_respuesta = payload_create(sizeof(uint32_t) + strlen(instruccion) + 1);

    payload_add_string(payload_respuesta, instruccion);

    paquete_t *paquete = crear_paquete(MEMORY_PID_PSEUDOCODE, payload_respuesta);

    enviar_paquete(conexion_cpu, paquete);

    payload_destroy(payload_respuesta);

    liberar_paquete(paquete);

    log_debug(extra_logger, "Proceso [PID: %d] envia linea N%d: %s", pid, programCounter, instruccion);
}

t_list *leer_pseudocodigo(String filename) {

    FILE* pseudocodigo = fopen(filename, "r");

    if (pseudocodigo == NULL) {
        fprintf(stderr, "Error al abrir el archivo de pseudocodigo! %s\n", filename);
        perror("");
        return NULL;
    }

    char linea[BUFF_SIZE];
    t_list* instrucciones = list_create();

    while (fgets(linea, sizeof(linea), pseudocodigo)) {

        if (linea[0] == '\n') // Ignora las lineas del archivo con solo salto de linea
           continue;

        linea[strcspn(linea, "\n")] = '\0';  // Elimina el salto de línea al final de la instrucción
        String instruccion = strdup(linea);  // Copia la linea leida en un string nuevo

        if (instruccion == NULL) {
            list_destroy_and_destroy_elements(instrucciones, free);
            return NULL;
        }

        list_add(instrucciones, instruccion); // Agrega la instrucción a la lista de instrucciones
    }

    fclose(pseudocodigo);
    return instrucciones;
}

void imprimir_instrucciones(t_list* instrucciones) {
    for (uint32_t i = 0; i < list_size(instrucciones); i++) {
        printf("Linea %d: ", i);
        imprimir_instruccion(list_get(instrucciones, i));
    }
}

void imprimir_instruccion(String instruccion) {
    if(instruccion == NULL)
        fprintf(stderr, "No existe la instrucción!\n");
    printf("%s\n", instruccion);
}

void imprimir_instruccion_numero(t_list* instrucciones, uint32_t numero_de_linea) {
    printf("Linea %d: ", numero_de_linea);
    imprimir_instruccion(list_get(instrucciones, numero_de_linea));
}

Proceso_t *proceso_get(uint16_t pid) {
    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pid); // Convierto el pid a string para poder usarlo como key en el diccionario
    return dictionary_get(procesos, str_pid);
}
