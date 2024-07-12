#include "consola.h"
#include "scheduler.h"
#include <errno.h>
#include "kernel_interface_handler.h"
#include "planificador_largo_plazo.h"
#include "recursos.h"
//#include "semaforos.h"

extern t_kernel_config* kernel_config;
extern t_log* logger;
extern t_log* extra_logger;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt;
extern sem_t sem_iniciar_planificacion, sem_proceso_a_new, sem_planificador_corto_comando, sem_planificador_largo_comando, sem_manejo_interrupciones_comando;

extern bool estado_planificacion_activa;
extern scheduler_t *scheduler;   // Estructura de planificacion general
extern t_dictionary *interfaces; // Guarda las interfaces conectadas al kernel
extern t_dictionary *instrucciones;
extern pthread_mutex_t diccionario_instrucciones_mutex;

uint16_t numero_de_procesos = 0;
uint16_t pid_a_finalizar = 0; // Necesario para las funciones estaticas

///////////// FUNCIONES PARA DEBUGEAR /////////////
static void print_io_name(void *key) {
    printf("[Interfaz: %s]\n", (String)key);
}

static void print_io(String arg) {
    t_list *lista_io = dictionary_keys(interfaces);
    list_iterate(lista_io, print_io_name);
    list_destroy(lista_io);
}
///////////// FUNCIONES PARA DEBUGEAR /////////////

void mensaje_de_bienvenida() {
    printf("---------------------------------------------------------------------------\n");
    printf("|                  Bienvenido a la Consola de Comandos!                   |\n");
    printf("---------------------------------------------------------------------------\n");
    printf("| Podes utilizar los siguientes comandos para controlar el sistema:       |\n");
    printf("|    EJECUTAR_SCRIPT [PATH]    - Ejecuta un script de instrucciones       |\n");
    printf("|    INICIAR_PROCESO [PATH]    - Inicia un nuevo proceso                  |\n");
    printf("|    FINALIZAR_PROCESO [PID]   - Termina un proceso existente             |\n");
    printf("|    DETENER_PLANIFICACION     - Detiene la planificación de proceso      |\n");
    printf("|    INICIAR_PLANIFICACION     - Reanuda la planificación de procesos     |\n");
    printf("|    MULTIPROGRAMACION [VALOR] - Modifica el grado de multiprogramación   |\n");
    printf("|    PROCESO_ESTADO            - Lista los procesos por estado            |\n");
    printf("|[D] PRINT_IO                  - Lista las interfaces conectadas [DEBUG]  |\n");
    printf("---------------------------------------------------------------------------\n");
    printf("| Escribe 'exit' para salir de la consola.                                |\n");
    printf("| Escribe 'help' para imprimir este mensaje nuevamente.                   |\n");
    printf("---------------------------------------------------------------------------\n");
    printf("\n> ");
}

void ejecutar_script(const String path) {

    if (path == NULL) {
        puts("Script invalido!");
        return;
    }

    char full_path[BUFF_SIZE];

    if (getcwd(full_path, sizeof(full_path)) == NULL) {
        perror("Error en getcwd()");
        exit(EXIT_FAILURE);
    }

    strcat(full_path, path);

    printf("Ejecutando script: %s\n", full_path);

    FILE *file = fopen(full_path, "r");

    if (file == NULL) {
        printf("No se pudo abrir el archivo en: %s\n", full_path);
        return;
    }

    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) {
        line[strcspn(line, "\r\n")] = 0;  // Elimina \r y \n del comando
        manejar_comando(line);
    }

    free(line);
    fclose(file);
}

void iniciar_proceso(const String path) {

    if (path == NULL) {
        puts("Proceso invalido!");
        return;
    }

    pcb_t *pcb = crear_proceso(++numero_de_procesos, kernel_config->quantum);

    if(pcb == NULL) {
        log_error(extra_logger, "No se pudo crear el PCB para %s", path);
        return;
    }

    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pcb->pid); // Convierto el pid a string para poder usarlo como key en el diccionario

    dictionary_put(scheduler->procesos, str_pid, pcb);
    
    log_debug(extra_logger, "PROCESO CREADO [PID: %s]", str_pid);

    crear_proceso_en_memoria(pcb->pid, path);

    proceso_a_cola_new(pcb); // Paso el proceso a la cola new

    log_info(logger, "Se crea el proceso %s en NEW", str_pid);
}

static void free_tokens(void *element) {
    char** tokens = (char **)element;
    if (tokens == NULL)
        return;
    free(tokens[0]);
    free(tokens);
}

static bool remover_proceso_lista(void* elemento) {
    pcb_t* proceso = (pcb_t*)elemento;
    return proceso->pid == pid_a_finalizar;
}

static void remover_proceso_diccionario(char *key, void *cola_bloqueada) {
    proceso_a_exit(list_remove_by_condition(cola_bloqueada, remover_proceso_lista), cola_bloqueada);

    if (dictionary_has_key(interfaces, key)) {
        char str_pid[8];
        snprintf(str_pid, sizeof(str_pid), "%d", pid_a_finalizar);
        pthread_mutex_lock(&diccionario_instrucciones_mutex);
        dictionary_remove_and_destroy(instrucciones, str_pid, free_tokens);
        pthread_mutex_unlock(&diccionario_instrucciones_mutex);
    }
}

void finalizar_proceso(const String str_pid) {

    if (str_pid == NULL) {
        puts("PID INVALIDO!");
        return;
    }

    if(dictionary_has_key(scheduler->procesos, str_pid) == false) {
        log_error(extra_logger, "PROCESO [PID: %s] NO EXISTE", str_pid);
        return;
    }

    pid_a_finalizar = atoi(str_pid);

    if(pid_a_finalizar == 0) {
        puts("PID INVALIDO!");
        return;
    }

    pthread_mutex_lock(&scheduler->mutex_exec);
    if (scheduler->proceso_ejecutando != NULL && scheduler->proceso_ejecutando->pid == pid_a_finalizar) {
        enviar_operacion(FINALIZADO, conexion_interrupt);
        pthread_mutex_unlock(&scheduler->mutex_exec);
        return;
    }    
    pthread_mutex_unlock(&scheduler->mutex_exec);

    pthread_mutex_lock(&scheduler->mutex_new);
    proceso_a_exit(list_remove_by_condition(scheduler->cola_new, remover_proceso_lista), scheduler->cola_new);
    pthread_mutex_unlock(&scheduler->mutex_new);

    pthread_mutex_lock(&scheduler->mutex_ready);
    proceso_a_exit(list_remove_by_condition(scheduler->cola_ready, remover_proceso_lista), scheduler->cola_ready);
    pthread_mutex_unlock(&scheduler->mutex_ready);

    pthread_mutex_lock(&scheduler->mutex_aux_blocked);
    proceso_a_exit(list_remove_by_condition(scheduler->cola_aux_blocked, remover_proceso_lista), scheduler->cola_aux_blocked);
    pthread_mutex_unlock(&scheduler->mutex_aux_blocked);

    pthread_mutex_lock(&scheduler->mutex_blocked);
    dictionary_iterator(scheduler->colas_blocked, remover_proceso_diccionario);
    pthread_mutex_unlock(&scheduler->mutex_blocked);

    finalizar_proceso_en_memoria(pid_a_finalizar);
    liberar_recursos_de_proceso(pid_a_finalizar);
    
    log_debug(extra_logger, "PROCESO FINALIZADO [PID: %d]", pid_a_finalizar);
    log_info(logger, "Finalizo proceso %d - Motivo: INTERRUPTED_BY_USER", pid_a_finalizar);
}

void iniciar_planificacion(const String s) {
    if (s != NULL) {
        puts("El comando no deberia contener argumentos!");
        return;
    }

    if (estado_planificacion_activa) {
        puts("El planificador ya esta activado");
        return;
    }

    estado_planificacion_activa = true;

    sem_post(&sem_planificador_corto_comando);
    sem_post(&sem_planificador_largo_comando);
    sem_post(&sem_manejo_interrupciones_comando);
    dictionary_iterator(interfaces, sem_post_interfaces);

    puts("Reanudando planificador");
}

void sem_post_interfaces(char *key, void *element){
    interfaz_t *interfaz = (interfaz_t*)element;
    sem_post(&interfaz->sem_interfaz_comando);
}

void detener_planificacion(const String s) {
    if (s != NULL) {
        puts("El comando no deberia contener argumentos!");
        return;
    }
    // Deberia ir un mutex en esta variable estado_planificacion_activa?
    estado_planificacion_activa = false;
    puts("Deteniendo planificador");
}

void multiprogramacion(const String valor) {

    if (valor == NULL) {
        log_error(extra_logger, "Ingresar un valor de multiprogramacion no nulo!");
        return;
    }

    char *resto;

    errno = 0;

    long multiprogramacion = strtol(valor, &resto, 10);

    if (errno != 0 || resto == valor || *resto != '\0' || multiprogramacion <= 0 || multiprogramacion > UINT16_MAX) {
        log_error(extra_logger, "Ingresar un grado de multiprogramacion valido! Entre 1 y %d", UINT16_MAX);
        return;
    }

    cambiar_grado_multiprogramacion(multiprogramacion);

    log_debug(extra_logger, "Cambiando multiprogramacion a %d\n", kernel_config->grado_multiprogramacion);

}

void proceso_estado(const String s) {
    if (s != NULL) {
        puts("El comando no deberia contener argumentos adicionales!");
        return;
    }
    puts("Listando procesos...\n");

    t_list* lista_de_procesos = dictionary_elements(scheduler->procesos);

    list_iterate(lista_de_procesos, imprimir_proceso);

    list_clean(lista_de_procesos);

    list_destroy(lista_de_procesos);

    return;
}

void imprimir_proceso(void* proceso) {
    pcb_t* pcb = (pcb_t*)proceso;
    char estado[8];
    switch (pcb->estado) {
        case NEW:
            strcpy(estado, "NEW");
            break;
        case READY:
            strcpy(estado, "READY");
            break;
        case EXEC:
            strcpy(estado, "EXEC");
            break;
        case BLOCKED:
            strcpy(estado, "BLOCKED");
            break;
        case EXIT:
            strcpy(estado, "EXIT");
            break;
        default:
            strcpy(estado, "UNKNOWN");
            break;
    }
    printf("[PID: %d | ESTADO: %s]\n", pcb->pid, estado);
}

void manejar_comando(const String comando) {

    String cmd = strtok(comando, " ");

    String arg = strtok(NULL, " ");

    if (cmd == NULL) {
        printf(">>Ingresar un comando!\n");
    } else if (strcmp(cmd, "EJECUTAR_SCRIPT") == 0) {
        ejecutar_script(arg);
    } else if (strcmp(cmd, "INICIAR_PROCESO") == 0) {
        iniciar_proceso(arg);
    } else if (strcmp(cmd, "FINALIZAR_PROCESO") == 0) {
        finalizar_proceso(arg);
    } else if (strcmp(cmd, "DETENER_PLANIFICACION") == 0) {
        detener_planificacion(arg);
    } else if (strcmp(cmd, "INICIAR_PLANIFICACION") == 0) {
        iniciar_planificacion(arg);
    } else if (strcmp(cmd, "MULTIPROGRAMACION") == 0) {
        multiprogramacion(arg);
    } else if (strcmp(cmd, "PROCESO_ESTADO") == 0) {
        proceso_estado(arg);
    } else if (strcmp(cmd, "PRINT_IO") == 0) { // Solo para debugear
        print_io(arg);
    } else {
        printf(">>Comando no reconocido!\n");
    }
}

void* thread_consola(void* arg) {

    char* command = NULL;
    size_t len = 0;

    mensaje_de_bienvenida();

    while (getline(&command, &len, stdin) != -1) {

        command[strcspn(command, "\r\n")] = '\0'; // Elimina \r y \n del comando

        if (strcmp(command, "exit") == 0) {
            destroy_scheduler(scheduler);
            dictionary_destroy_and_destroy_elements(interfaces, free);
            break;
        }

        if (strcmp(command, "help") == 0) {
            mensaje_de_bienvenida();
            continue;
        }

        manejar_comando(command);

        printf("> ");
    }

    puts("\nSaliendo de la consola...");

    free(command); // Liberar la memoria asignada por getline

    return NULL;
}

int consola_kernel() {

    pthread_t hilo;
    
    if (pthread_create(&hilo, NULL, thread_consola, NULL) != 0) {
        perror("No se pudo crear el hilo para manejar la consola");
        exit(EXIT_FAILURE);
    }

    pthread_join(hilo, NULL);

    return OK;
}

void crear_proceso_en_memoria(uint16_t pid, String path) {
    uint32_t total_size = sizeof(uint16_t) +  // pid
                          sizeof(uint32_t) +  // Para indicar la longitud del string que viene en el payload
                          strlen(path) + 1;   // String + \0

    payload_t *payload = payload_create(total_size);
    payload_add(payload, &pid, sizeof(uint16_t));
    payload_add_string(payload, path);
    paquete_t *paquete = crear_paquete(MEMORY_PROCESS_CREATE, payload);

    if(enviar_paquete(conexion_memoria, paquete) != OK)
        exit(EXIT_FAILURE);
    
    payload_destroy(payload);
    liberar_paquete(paquete);
}

void finalizar_proceso_en_memoria(uint16_t pid) {

    payload_t *payload = payload_create(sizeof(uint16_t));
    payload_add(payload, &pid, sizeof(uint16_t));
    paquete_t *paquete = crear_paquete(MEMORY_PROCESS_TERM, payload);

    if(enviar_paquete(conexion_memoria, paquete) != OK)
        exit(EXIT_FAILURE);
    
    payload_destroy(payload);
    liberar_paquete(paquete);
}
