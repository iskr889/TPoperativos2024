#include "main.h"
#include "memory_interface_handler.h"

void* user_memory; // Espacio contiguo de memoria
t_bitarray* frame_bitarray;  // Bitarray para seguir el estado de los marcos

t_config* config;
t_memoria_config* memoria_config;
t_log* logger;
t_log* extra_logger;

int conexion_cpu, conexion_kernel, memoria_server;

int main(int argc, char* argv[]) {

    memoria_config = load_memoria_config("memoria.config");

    logger = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);

    extra_logger = iniciar_logger("memoria_debug.log", "MEMORIA", 1, LOG_LEVEL_DEBUG);

    // Iniciamos servidor escuchando por conexiones de CPU, KERNEL e INTERFACES
    memoria_server = escuchar_conexiones_de("CPU, KERNEL e INTERFACES", memoria_config->puerto_escucha, extra_logger);

    // La MEMORIA espera que la CPU se conecte
    conexion_cpu = esperar_conexion_de(CPU_CON_MEMORIA, memoria_server);

    if(conexion_cpu < 0)
        log_error(extra_logger, "MODULO CPU NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_debug(extra_logger, "MODULO CPU CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // La MEMORIA espera que el KERNEL se conecte
    conexion_kernel = esperar_conexion_de(KERNEL_CON_MEMORIA, memoria_server);

    if(conexion_kernel < 0)
        log_error(extra_logger, "MODULO KERNEL NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_debug(extra_logger, "MODULO KERNEL CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // Acepto interfaces en un thread aparte asi no frena la ejecución del programa
    manejador_de_interfaces(memoria_server);

    sleep(30);

    liberar_memoria();

    puts("\nCerrando Memoria...");
    
    return OK;
}

void liberar_memoria() {
    close(memoria_server);
    log_destroy(logger);
    log_destroy(extra_logger);
    config_destroy(config);
    free(memoria_config);
}

t_memoria_config* load_memoria_config(String path) {

    config = iniciar_config(path);

    if(config == NULL) {
        fprintf(stderr, "Config invalido!\n");
        exit(EXIT_FAILURE);
    }
    
    t_memoria_config* memoria_config = malloc(sizeof(t_memoria_config));
    
    if(memoria_config == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }

    memoria_config->puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    memoria_config->tam_memoria = config_get_string_value(config,"TAM_MEMORIA");
    memoria_config->tam_pagina = config_get_string_value(config,"TAM_PAGINA");
    memoria_config->path_intrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
    memoria_config->retardo_respuesta = config_get_string_value(config,"RETARDO_RESPUESTA");

    return memoria_config;
}

void inicializar_memoria() {

    user_memory = malloc(MEMORY_SIZE);

    if(user_memory == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }

    size_t bitmap_size = (MEMORY_SIZE / PAGE_SIZE) / CHAR_BIT;

    char* bitarray_data = calloc(bitmap_size, sizeof(char));

    if(bitarray_data == NULL) {
        perror("Error en calloc()");
        exit(EXIT_FAILURE);
    }

    frame_bitarray = bitarray_create_with_mode(bitarray_data, bitmap_size, LSB_FIRST);
}

// Crear un proceso en memoria
Proceso_t* crear_proceso(int pid, int cant_paginas) {

    Proceso_t* proceso = malloc(sizeof(Proceso_t));
    proceso->pid = pid;
    proceso->cant_paginas = cant_paginas;
    proceso->pagina = malloc(cant_paginas * sizeof(Page_table_t));

    if(proceso->pagina == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < cant_paginas; i++) {
        proceso->pagina[i].marco = -1; // Al crearse el proceso no tiene marcos asignados a la memoria
        proceso->pagina[i].asignada = false;
    }

    return proceso;
}

// Asignar páginas a un proceso
int asignar_pagina(Proceso_t* proceso) {

    int paginas_asignadas = 0;

    for (int i = 0; i < bitarray_get_max_bit(frame_bitarray); i++) {

        if (bitarray_test_bit(frame_bitarray, i) == false) {

            proceso->pagina[paginas_asignadas].marco = i;
            proceso->pagina[paginas_asignadas].asignada = true;
            bitarray_set_bit(frame_bitarray, i);
            paginas_asignadas++;

            if (paginas_asignadas == proceso->cant_paginas)
                return OK;
        }
    }

    return ERROR;
}

// Liberar páginas de un proceso
void liberar_paginas_proceso(Proceso_t* proceso) {

    for (int i = 0; i < proceso->cant_paginas; i++) {
        if (proceso->pagina[i].asignada)
            bitarray_clean_bit(frame_bitarray, proceso->pagina[i].marco);
    }

    free(proceso->pagina);
    free(proceso);
}

// Acceso a la tabla de páginas
int acceder_marco(Proceso_t* proceso, int numero_de_pagina) {

    if (numero_de_pagina >= proceso->cant_paginas)
        return ERROR;

    return proceso->pagina[numero_de_pagina].marco;
}

// Lectura desde el espacio de usuario
int leer_memoria(int direccion_fisica, void* buffer, size_t size) {

    if (direccion_fisica + size > MEMORY_SIZE) {
        fprintf(stderr, "Lectura fuera de limites!\n");
        return ERROR;
    }

    memcpy(buffer, user_memory + direccion_fisica, size);
    return OK;
}

// Escritura en el espacio de usuario
int escribir_memoria(int direccion_fisica, void* data, size_t size) {

    if (direccion_fisica + size > MEMORY_SIZE) {
        fprintf(stderr, "Escritura fuera de limites\n");
        return ERROR;
    }

    memcpy(user_memory + direccion_fisica, data, size);
    return OK;
}

// Ajustar tamaño de un proceso
int resize_proceso(Proceso_t* proceso, int new_cant_paginas) {

    if (new_cant_paginas > proceso->cant_paginas) { // Ampliación del proceso

        int paginas_adicionales = new_cant_paginas - proceso->cant_paginas;

        for (int i = 0; i < bitarray_get_max_bit(frame_bitarray) && paginas_adicionales > 0; i++) {

            if (!bitarray_test_bit(frame_bitarray, i)) {

                proceso->pagina = realloc(proceso->pagina, new_cant_paginas * sizeof(Page_table_t));
                proceso->pagina[proceso->cant_paginas].marco = i;
                proceso->pagina[proceso->cant_paginas].asignada = true;
                bitarray_set_bit(frame_bitarray, i);
                proceso->cant_paginas++;
                paginas_adicionales--;
            }
        }

        return (paginas_adicionales == 0) ? OK : ERROR; // ERROR: sin más memoria
    }
    
    if (new_cant_paginas < proceso->cant_paginas) { // Reducción del proceso

        for (int i = proceso->cant_paginas - 1; i >= new_cant_paginas; i--) {
            if (proceso->pagina[i].asignada)
                bitarray_clean_bit(frame_bitarray, proceso->pagina[i].marco);
        }

        proceso->pagina = realloc(proceso->pagina, new_cant_paginas * sizeof(Page_table_t));
        proceso->cant_paginas = new_cant_paginas;
    }

    return OK;
}
