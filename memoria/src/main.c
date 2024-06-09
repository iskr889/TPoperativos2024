#include "main.h"
#include "memory_interface_handler.h"
#include "manejar_instrucciones.h"

void* user_memory; // Espacio contiguo de memoria
char *bitarray_data;  // Necesario como global para que no quede sin liberar porque bitarray_destroy() no lo libera
t_bitarray* frame_bitarray;  // Bitarray para seguir el estado de los marcos
uint32_t paginas_totales; // Cantidad total de paginas en la memoria = TAM_MEMORIA / TAM_PAGINA
t_dictionary *procesos; // Guarda los procesos creados en memoria

t_config* config;
t_memoria_config* memoria_config;
t_log* logger;
t_log* extra_logger;

int conexion_cpu, conexion_kernel, memoria_server;

int main(int argc, char* argv[]) {

    memoria_config = load_memoria_config("memoria.config");

    logger = iniciar_logger("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);

    extra_logger = iniciar_logger("memoria_debug.log", "MEMORIA", 1, LOG_LEVEL_DEBUG);

    inicializar_memoria();

    // Iniciamos servidor escuchando por conexiones de CPU, KERNEL e INTERFACES
    memoria_server = escuchar_conexiones_de("CPU, KERNEL e INTERFACES", memoria_config->puerto_escucha, extra_logger);

    // La MEMORIA espera que la CPU se conecte
    conexion_cpu = esperar_conexion_de(CPU_CON_MEMORIA, memoria_server);

    if (conexion_cpu < 0)
        log_error(extra_logger, "MODULO CPU NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_debug(extra_logger, "MODULO CPU CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // La MEMORIA espera que el KERNEL se conecte
    conexion_kernel = esperar_conexion_de(KERNEL_CON_MEMORIA, memoria_server);

    if (conexion_kernel < 0)
        log_error(extra_logger, "MODULO KERNEL NO PUDO CONECTARSE CON LA MEMORIA!");
    else
        log_debug(extra_logger, "MODULO KERNEL CONECTO CON LA MEMORIA EXITOSAMENTE!");

    // Acepto interfaces en un thread aparte asi no frena la ejecución del programa
    manejador_de_interfaces(memoria_server);

    manejar_instrucciones_kernel();

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
    free(user_memory);
    free(bitarray_data);
    bitarray_destroy(frame_bitarray);
    // TODO: Deberia recorrer cada proceso para liberar la pagina y lista de instrucciones pero por ahora queda asi porque no hay tiempo :(
    dictionary_destroy_and_destroy_elements(procesos, free);
}

t_memoria_config* load_memoria_config(String path) {

    config = iniciar_config(path);

    if (config == NULL) {
        fprintf(stderr, "Config invalido!\n");
        exit(EXIT_FAILURE);
    }
    
    t_memoria_config* memoria_config = malloc(sizeof(t_memoria_config));
    
    if (memoria_config == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }

    memoria_config->puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    memoria_config->tam_memoria = config_get_int_value(config,"TAM_MEMORIA");
    memoria_config->tam_pagina = config_get_int_value(config,"TAM_PAGINA");
    memoria_config->path_intrucciones = config_get_string_value(config,"PATH_INSTRUCCIONES");
    memoria_config->retardo_respuesta = config_get_int_value(config,"RETARDO_RESPUESTA");

    if (memoria_config->tam_memoria <= 0 || memoria_config->tam_pagina <= 0 || memoria_config->retardo_respuesta <= 0) {
        fprintf(stderr, "Tamaño de memoria, pagina o retardo de respuesta invalido!\n");
        exit(EXIT_FAILURE);
    }

    return memoria_config;
}

void inicializar_memoria() {

    user_memory = malloc(memoria_config->tam_memoria);

    if (user_memory == NULL) {
        perror("Error en malloc()");
        exit(EXIT_FAILURE);
    }

    paginas_totales = memoria_config->tam_memoria / memoria_config->tam_pagina;

    if (paginas_totales <= 0)  {
        fprintf(stderr, "La cantidad de paginas totales tiene que ser mayor a cero! Por lo que: Tamaño memoria >= Tamaño pagina");
        exit(EXIT_FAILURE);
    }

    size_t bitarray_size = paginas_totales / CHAR_BIT; // Necesario porque las funciones usan Bytes no Bits!

    if (bitarray_size == 0)
        bitarray_size = 1; // Al menos necesito un bitarray de 1 byte

    bitarray_data = calloc(bitarray_size, sizeof(char)); // Inicializa la memoria en 0

    if (bitarray_data == NULL) {
        perror("Error en calloc()");
        exit(EXIT_FAILURE);
    }

    frame_bitarray = bitarray_create_with_mode(bitarray_data, bitarray_size, LSB_FIRST);

    procesos = dictionary_create();
}

void crear_proceso(uint16_t pid, t_list *instrucciones) {

    Proceso_t* proceso = malloc(sizeof(Proceso_t));
    proceso->pid = pid;
    proceso->paginas = list_create();
    proceso->instrucciones = instrucciones;
    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pid); // Convierto el pid a string para poder usarlo como key en el diccionario
    dictionary_put(procesos, str_pid, proceso);
}

void liberar_proceso(uint16_t pid) {

    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pid); // Convierto el pid a string para poder usarlo como key en el diccionario
    Proceso_t* proceso = dictionary_get(procesos, str_pid);
    if (proceso == NULL)
        return;
    list_destroy_and_destroy_elements(proceso->paginas, liberar_pagina);
    list_destroy_and_destroy_elements(proceso->instrucciones, free);
    dictionary_remove_and_destroy(procesos, str_pid, free);
}

void liberar_pagina(void *page) {
    PageTable_t *pagina = (PageTable_t *)page;
    if (pagina->asignada)
            bitarray_clean_bit(frame_bitarray, pagina->marco);
    free(pagina);
}

void liberar_paginas(t_list* paginas) {
    list_iterate(paginas, liberar_pagina);
}

bool asignar_paginas(Proceso_t* proceso, uint32_t cant_paginas) {

    if(cant_paginas == 0)
        return true;

    for (uint32_t i = 0, paginas_asignadas = 0; i < bitarray_get_max_bit(frame_bitarray); i++) { // Recorre todo el bitarray

        if (bitarray_test_bit(frame_bitarray, i) == false) { // Devuelve false si el frame no esta asignado a la memoria

            PageTable_t* nueva_pagina = malloc(sizeof(PageTable_t));

            if (nueva_pagina == NULL)
                return false;

            nueva_pagina->marco = i;
            nueva_pagina->asignada = true;
            list_add(proceso->paginas, nueva_pagina);
            bitarray_set_bit(frame_bitarray, i);

            if (++paginas_asignadas >= cant_paginas)
                return true;
        }
    }

    return false;
}

int acceder_marco(Proceso_t* proceso, uint32_t numero_de_pagina) {

    if (numero_de_pagina >= list_size(proceso->paginas))
        return ERROR;

    PageTable_t *pagina = list_get(proceso->paginas, numero_de_pagina);
    return pagina->marco;
}

bool resize_process(Proceso_t* proceso, uint32_t nueva_cant_paginas) {

    uint32_t cant_paginas = list_size(proceso->paginas);

    if (nueva_cant_paginas > cant_paginas) // Ampliación del proceso
        return asignar_paginas(proceso, nueva_cant_paginas - cant_paginas);

    if (nueva_cant_paginas < cant_paginas) { // Reducción del proceso
        for (int i = cant_paginas - 1; i >= nueva_cant_paginas; i--) {
            PageTable_t *pagina = list_remove(proceso->paginas, i);
            liberar_pagina(pagina);
        }
    }

    return true;
}

bool leer_memoria(uint32_t direccion_fisica, void *buffer, size_t size) {

    if (direccion_fisica + size > memoria_config->tam_memoria) {
        log_debug(extra_logger, "Lectura de memoria fisica fuera de limites!");
        return false;
    }

    memcpy(buffer, user_memory + direccion_fisica, size);
    return true;
}

bool escribir_memoria(uint32_t direccion_fisica, void *data, size_t size) {

    if (direccion_fisica + size > memoria_config->tam_memoria) {
        log_debug(extra_logger, "Escritura de memoria fisica fuera de limites!");
        return false;
    }

    memcpy(user_memory + direccion_fisica, data, size);
    return true;
}
