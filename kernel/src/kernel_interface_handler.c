#include "kernel_interface_handler.h"
#include "manejo_interrupciones_cpu.h"
#include "scheduler.h"

extern scheduler_t *scheduler;
t_dictionary *interfaces;
t_dictionary *instrucciones;
extern bool VRR_modo;
extern bool estado_planificacion_activa;
extern t_log* extra_logger;
extern sem_t sem_hay_encolado_VRR;
pthread_mutex_t diccionario_instrucciones_mutex = PTHREAD_MUTEX_INITIALIZER;

void manejar_interfaces(int fd_servidor) {

    pthread_t thread_interfaces;
    
    instrucciones = dictionary_create();

    int* fd_servidor_p = malloc(sizeof(int));

    if (fd_servidor_p == NULL) {
        perror("Error en malloc()");
        exit(ERROR);
    }

    *fd_servidor_p = fd_servidor;

    if (pthread_create(&thread_interfaces, NULL, thread_aceptar_interfaces, fd_servidor_p) != 0) {
        perror("No se pudo crear el hilo para manejar interfaces");
        free(fd_servidor_p);
        exit(EXIT_FAILURE);
    }

    pthread_detach(thread_interfaces);

    return;
}

void *thread_aceptar_interfaces(void *socket_servidor) {
    interfaces = dictionary_create();
    aceptar_interfaces(*(int*)socket_servidor);
    free(socket_servidor);
    dictionary_destroy_and_destroy_elements(interfaces, free);
    pthread_exit(NULL);
    return NULL;
}

int aceptar_interfaces(int socket_servidor) {

    while(1) {

        int* socket_interfaz = malloc(sizeof(int));

        if (socket_interfaz == NULL) {
            perror("Error en malloc()");
            return ERROR;
        }

        *socket_interfaz = accept(socket_servidor, NULL, NULL);

        if (*socket_interfaz < 0) {
            perror("Error en accept()");
            free(socket_interfaz); // Libera la memoria en caso de error
            return ERROR;
        }

        pthread_t thread;

        if (pthread_create(&thread, NULL, thread_handshake_con_interfaz, socket_interfaz)) {
            perror("Error en pthread_create()");
            close(*socket_interfaz); // Cierro el socket del interfaz en caso de error
            free(socket_interfaz); // Libera la memoria en caso de error
            return ERROR;
        }

        pthread_detach(thread);
    }

    close(socket_servidor);

    return OK;
}

int handshake_con_interfaz(int socket_interfaz) {
    conexion_t tipo_handshake = HANDSHAKE_ERROR;
    int32_t rta_handshake = 0;

    if(recv(socket_interfaz, &tipo_handshake, sizeof(tipo_handshake), MSG_WAITALL) < 0)
        return ERROR;

    if(tipo_handshake < 0 || tipo_handshake >= HANDSHAKE_ERROR)
        rta_handshake = -1;

    ssize_t bytes_send = send(socket_interfaz, &rta_handshake, sizeof(rta_handshake), 0);

    if(bytes_send < 0 || rta_handshake != 0)
        return ERROR;

    if(manejar_interfaz(tipo_handshake, socket_interfaz) < 0)
        return ERROR;

    return OK;
}

void* thread_handshake_con_interfaz(void* fd_interfaz) {
    int conexion = *(int*)fd_interfaz;
    free(fd_interfaz);
    if(handshake_con_interfaz(conexion) < 0) {
        perror("Error en el handshake con la Interfaz!");
        close(conexion);
    }
    pthread_exit(NULL);
}

static void free_tokens(void *element) {
    char** tokens = (char **)element;
    if (tokens == NULL)
        return;
    free(tokens);
}

int manejar_interfaz(conexion_t handshake, int socket_interfaz) {

    String nombre = recibir_nombre_interfaz(socket_interfaz);
    interfaz_t *interfaz = malloc(sizeof(interfaz_t));
    interfaz->socket = socket_interfaz;
    sem_init(&interfaz->sem_IO_ejecucion, 0, 0);
    sem_init(&interfaz->sem_interfaz_comando, 0, 0);
    pthread_mutex_init(&interfaz->mutex_IO_instruccion, NULL);
    interfaz->instruccion_IO = list_create();

    switch (handshake) {
        case GENERIC_CON_KERNEL:
            interfaz->tipo = GENERIC;
            printf("Interfaz GENERICA conectada con el KERNEL [%s]\n> ", nombre);
            break;
        case STDIN_CON_KERNEL:
            interfaz->tipo = STDIN;
            printf("Interfaz STDIN conectada con el KERNEL [%s]\n> ", nombre);
            //send_io_stdin_read(socket_interfaz, 12, 4); // Prueba
            break;
        case STDOUT_CON_KERNEL:
            interfaz->tipo = STDOUT;
            printf("Interfaz STDOUT conectada con el KERNEL [%s]\n> ", nombre);
            //send_io_stdout_write(socket_interfaz, 12, 4); // Prueba
            break;
        case DIALFS_CON_KERNEL:
            interfaz->tipo = DIALFS;
            printf("Interfaz DIALFS conectada con el KERNEL [%s]\n> ", nombre);
            break;
        default:
            fprintf(stderr, "Error al tratar de identificar el handshake de la IO conectada!\n> ");
            free(nombre);
            return ERROR;
    }

    fflush(stdout);
    dictionary_put(interfaces, nombre, interfaz); // Guardo la conexión en un diccionario y uso el nombre de la interfaz como key

    while(1){
        
        sem_wait(&interfaz->sem_IO_ejecucion);
        log_info(extra_logger, "Paso el wait IO");

        pthread_mutex_lock(&scheduler->mutex_blocked);
        t_list *cola_bloqueados = dictionary_get(scheduler->colas_blocked, nombre);
        pcb_t *pcb = list_get(cola_bloqueados, 0); //Obtengo el primer elemento de la cola
        pthread_mutex_unlock(&scheduler->mutex_blocked);

        char str_pid[8];
        snprintf(str_pid, sizeof(str_pid), "%d", pcb->pid);
        pthread_mutex_lock(&diccionario_instrucciones_mutex);
        char **instruccion = dictionary_get(instrucciones, str_pid);
        pthread_mutex_unlock(&diccionario_instrucciones_mutex);

        ejecutar_IO(socket_interfaz, pcb->pid, instruccion);

        if (!estado_planificacion_activa) {
            sem_wait(&interfaz->sem_interfaz_comando);
        }
 
        log_info(extra_logger, "Ejecuto la IO");
        if (VRR_modo) {
            cola_blocked_a_aux_blocked(nombre);
            sem_post(&sem_hay_encolado_VRR); //Si es VRR
            log_info(extra_logger, "Paso la cosa a aux blocked");
        } else {
            cola_blocked_a_ready(nombre);
        }

        pthread_mutex_lock(&diccionario_instrucciones_mutex);
        dictionary_remove_and_destroy(instrucciones, str_pid, free_tokens);
        pthread_mutex_unlock(&diccionario_instrucciones_mutex);

    }

    free(nombre);
    return OK;
}

String recibir_nombre_interfaz(int socket) {

    paquete_t *paquete = recibir_paquete(socket);

    if(paquete == NULL)
        exit(EXIT_FAILURE);

    String nombre = payload_read_string(paquete->payload);

    payload_destroy(paquete->payload);
    liberar_paquete(paquete);

    return nombre;
}

void send_io_gen_sleep(int socket, uint16_t pid, uint32_t tiempo) {

    payload_t *payload = payload_create(sizeof(uint32_t));

    payload_add(payload, &pid, sizeof(uint16_t));
    payload_add(payload, &tiempo, sizeof(uint32_t));

    paquete_t *paquete = crear_paquete(IO_GEN_SLEEP, payload);

    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);

    payload_destroy(payload);
    liberar_paquete(paquete);
}

void send_io_stdin_read(int socket, uint16_t pid, uint32_t direccion, uint32_t cant_caracteres) {

    payload_t *payload = payload_create(sizeof(uint32_t) + sizeof(uint32_t));

    payload_add(payload, &pid, sizeof(uint16_t));
    payload_add(payload, &direccion, sizeof(uint32_t));
    payload_add(payload, &cant_caracteres, sizeof(uint32_t));

    paquete_t *paquete = crear_paquete(IO_STDIN_READ, payload);

    if(enviar_paquete(socket, paquete) != OK){
        printf("fallo al enviar el paquete a stdin");
        exit(EXIT_FAILURE);
    }

    payload_destroy(payload);
    liberar_paquete(paquete);
}

void send_io_stdout_write(int fd_io, uint16_t pid, uint32_t direccion, uint32_t cant_caracteres) {

    payload_t *payload = payload_create(sizeof(uint32_t) + sizeof(uint32_t));

    payload_add(payload, &pid, sizeof(uint16_t));
    payload_add(payload, &direccion, sizeof(uint32_t));
    payload_add(payload, &cant_caracteres, sizeof(uint32_t));

    paquete_t *paquete = crear_paquete(IO_STDOUT_WRITE, payload);

    if(enviar_paquete(fd_io, paquete) != OK){
        printf("fallo al enviar el paquete a stdin");
        exit(EXIT_FAILURE);
    }

    payload_destroy(payload);
    liberar_paquete(paquete);
}