#include "consola.h"
#include "scheduler.h"
#include <stdint.h>
#include <errno.h>

extern t_kernel_config* kernel_config;
extern t_log* logger;
extern t_log* extra_logger;
extern int conexion_memoria, conexion_dispatch, conexion_interrupt;

extern scheduler_t *scheduler;   // Estructura de planificacion general
extern t_dictionary *interfaces; // Guarda las interfaces conectadas al kernel

uint16_t numero_de_procesos = 0;

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
    printf("|    EJECUTAR_SCRIPT <PATH>    - Ejecuta un script de instrucciones       |\n");
    printf("|    INICIAR_PROCESO <PATH>    - Inicia un nuevo proceso                  |\n");
    printf("|    FINALIZAR_PROCESO <PATH>  - Termina un proceso existente             |\n");
    printf("|    DETENER_PLANIFICACION     - Detiene la planificación de proceso      |\n");
    printf("|    INICIAR_PLANIFICACION     - Reanuda la planificación de procesos     |\n");
    printf("|    MULTIPROGRAMACION         - Modifica el grado de multiprogramación   |\n");
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
    printf("Ejecutando script: %s\n", path);
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

    enviar_proceso_memoria(pcb->pid, path);
}

void finalizar_proceso(const String str_pid) {
    if (str_pid == NULL) {
        puts("PID INVALIDO!");
        return;
    }
    int pid = atoi(str_pid); // Modificar o usar strtol()
    printf("Deteniendo proceso: %d\n", pid);
}

void iniciar_planificacion(const String s) {
    if (s != NULL) {
        puts("El comando no deberia contener argumentos!");
        return;
    }
    puts("Reanudando planificador");
}

void detener_planificacion(const String s) {
    if (s != NULL) {
        puts("El comando no deberia contener argumentos!");
        return;
    }
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

    kernel_config->grado_multiprogramacion = multiprogramacion;

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

void enviar_proceso_memoria(uint16_t pid, String path) {
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
