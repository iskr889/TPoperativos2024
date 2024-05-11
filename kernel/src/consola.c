#include "consola.h"

void exec_script(const String script_name) {
    if (script_name == NULL) {
        puts("Script invalido!");
        return;
    }
    printf("Ejecutando script: %s\n", script_name);
}

void init_process(const String process_name) {
    if (process_name == NULL) {
        puts("Proceso invalido!");
        return;
    }
    printf("Iniciando proceso: %s\n", process_name);
}

void kill_process(const String str_pid) {
    if (str_pid == NULL) {
        puts("PID INVALIDO!");
        return;
    }
    int pid = atoi(str_pid); // Modificar o usar strtol()
    printf("Deteniendo proceso: %d\n", pid);
}

void resume_scheduling(const String arg) {
    if (arg != NULL) {
        puts("El comando no deberia terner argumentos adicionales!");
        return;
    }
    puts("Reanudando planificador");
}

void stop_scheduling(const String arg) {
    if (arg != NULL) {
        puts("El comando no deberia terner argumentos adicionales!");
        return;
    }
    puts("Deteniendo planificador");
}

void list_processes(const String arg) {
    if (arg != NULL) {
        puts("El comando no deberia terner argumentos adicionales!");
        return;
    }
    puts("Listando procesos...");
}

// Función para el manejo de comandos
void manejar_comando(const String comando) {

    String cmd = strtok(comando, " ");
    String arg = strtok(NULL, " ");

    if (strcmp(cmd, "EJECUTAR_SCRIPT") == 0) {
        exec_script(arg);
    } else if (strcmp(cmd, "INICIAR_PROCESO") == 0) {
        init_process(arg);
    } else if (strcmp(cmd, "FINALIZAR_PROCESO") == 0) {
        kill_process(arg);
    } else if (strcmp(cmd, "DETENER_PLANIFICACION") == 0) {
        resume_scheduling(arg);
    } else if (strcmp(cmd, "INICIAR_PLANIFICACION") == 0) {
        stop_scheduling(arg);
    } else if (strcmp(cmd, "PROCESO_ESTADO") == 0) {
        list_processes(arg);
    } else {
        printf("Comando no reconocido.\n");
    }
}

// Thread para la interfaz de usuario
void* thread_consola(void* arg) {

    char* command = NULL;
    size_t len = 0;
    ssize_t nread;

    printf("Consola: ");

    while ((nread = getline(&command, &len, stdin)) != -1) { // Reemplazar por do-while
        if (command[nread - 1] == '\n')
            command[nread - 1] = '\0'; // Eliminar el salto de línea
        if (strcmp(command, "exit") == 0)
            break;
        manejar_comando(command);
        printf("Consola: ");
    }

    puts("Saliendo de la consola...");

    free(command); // No olvidar liberar la memoria asignada por getline

    pthread_exit(NULL);

    return NULL;
}


// Iniciar el thread_consola
int consola_kernel() {

    pthread_t hilo;
    
    if (pthread_create(&hilo, NULL, thread_consola, NULL) != 0) {
        perror("No se pudo crear el hilo para manejar la consola");
        exit(EXIT_FAILURE);
    }

    pthread_detach(hilo);

    return EXIT_OK;
}