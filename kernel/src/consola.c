#include "consola.h"

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
    printf("Iniciando proceso: %s\n", path);
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
        puts("VALOR INVALIDO!");
        return;
    }
    int multiprogramacion = atoi(valor); // Modificar o usar strtol()
    printf("Cambiando multiprogramacion a %d\n", multiprogramacion);
}

void proceso_estado(const String s) {
    if (s != NULL) {
        puts("El comando no deberia contener argumentos adicionales!");
        return;
    }
    puts("Listando procesos...");
}

// Función para el manejo de comandos
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
    } else {
        printf(">>Comando no reconocido!\n");
    }
}

// Thread para la interfaz de usuario
void* thread_consola(void* arg) {

    char* command = NULL;
    size_t len = 0;

    mensaje_de_bienvenida();

    while (getline(&command, &len, stdin) != -1) {

        command[strcspn(command, "\r\n")] = '\0'; // Elimina \r y \n del comando

        if (strcmp(command, "exit") == 0)
            break;

        if (strcmp(command, "help") == 0) {
            mensaje_de_bienvenida();
            continue;
        }

        manejar_comando(command);

        printf("> ");
    }

    puts("\nSaliendo de la consola...");

    free(command); // No olvidar liberar la memoria asignada por getline

    return NULL;
}


// Iniciar el thread_consola
int consola_kernel() {

    pthread_t hilo;
    
    if (pthread_create(&hilo, NULL, thread_consola, NULL) != 0) {
        perror("No se pudo crear el hilo para manejar la consola");
        exit(EXIT_FAILURE);
    }

    pthread_join(hilo, NULL);

    return OK;
}