#include "manejar_instrucciones.h"

extern t_log* logger;
extern t_log* extra_logger;

extern int conexion_kernel;

void *thread_instrucciones_kernel(void *arg) {

    while(1) {

        paquete_t *paquete = recibir_paquete(conexion_kernel);

        if(paquete == NULL)
            exit(EXIT_FAILURE);

        switch (paquete->operacion) {
            case MEMORY_PROCESS_CREATE:
                instruccion_process_create(paquete->payload);
                break;
            case MEMORY_PROCESS_TERM:
                puts("Instrucción: MEMORY_PROCESS_TERM");
                break;
            case MEMORY_PAGE_TABLE_ACCESS:
                puts("Instrucción: MEMORY_PAGE_TABLE");
                break;
            case MEMORY_PROCESS_RESIZE:
                puts("Instrucción: MEMORY_PROCESS_RESIZE");
                break;
            default:
                puts("Error al tratar de identificar el codigo de operación del kernel!\n");
                return arg;
        }
    }

    return arg;
}

void manejar_instrucciones_kernel() {
    pthread_t hilo_instrucciones_kernel;

    if (pthread_create(&hilo_instrucciones_kernel, NULL, thread_instrucciones_kernel, NULL) != 0) {
        perror("No se pudo crear el hilo de instrucciones kernel");
        exit(EXIT_FAILURE);
    }

    pthread_join(hilo_instrucciones_kernel, NULL);
}


void instruccion_process_create(payload_t* payload) {

    char pseudocodigo[BUFF_SIZE] = CARPETA_PSEUDOCODIGO;

    uint16_t pid;

    payload_read(payload, &pid, sizeof(uint16_t));

    char str_pid[8];

    snprintf(str_pid, sizeof(str_pid), "%d", pid); // Convierto el pid a string para poder usarlo como key en el diccionario

    String path = payload_read_string(payload);

    strcat(pseudocodigo, path); // Concateno el directorio de la carpeta de pseudocodigo

    free(path);

    t_list *instrucciones = leer_pseudocodigo(pseudocodigo);

    if(instrucciones == NULL) {
        puts("Path invalido!");
        return;
    }

    crear_proceso(pid, instrucciones);

    imprimir_instrucciones(instrucciones);

    log_debug(extra_logger, "CREAR PROCESO RECIBIDO [PID: %s] PSEUDOCODIGO EN [PATH: %s]", str_pid, pseudocodigo);
}

t_list *leer_pseudocodigo(String filename) {

    FILE* pseudocodigo = fopen(filename, "r");

    if (pseudocodigo == NULL) {
        perror("Error al abrir el archivo de pseudocodigo!");
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
    printf("Enviando instrucción a la CPU: %s\n", instruccion);
}

void imprimir_instruccion_numero(t_list* instrucciones, uint32_t numero_de_linea) {
    printf("CPU pidio Linea %d: ", numero_de_linea);
    imprimir_instruccion(list_get(instrucciones, numero_de_linea));
}
