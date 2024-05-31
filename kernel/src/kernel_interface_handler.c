#include "kernel_interface_handler.h"

extern t_log* extra_logger;
t_dictionary *interfaces;

void manejador_de_interfaces(int fd_servidor) {

    pthread_t thread_interfaces;
        
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
            continue;
        }

        pthread_t thread;

        if (pthread_create(&thread, NULL, thread_handshake_con_interfaz, socket_interfaz)) {
            perror("Error en pthread_create()");
            close(*socket_interfaz); // Cierro el socket del interfaz en caso de error
            free(socket_interfaz); // Libera la memoria en caso de error
            continue;
        }

        pthread_detach(thread);
    }

    close(socket_servidor);

    return OK;
}

// Este handshake se ejecuta en un hilo que es creado despues del accept() y es unico por cada modulo conectado
int handshake_con_interfaz(int socket_interfaz) {
    conexion_t tipo_handshake = HANDSHAKE_ERROR; // Esto es un enum que identifica el tipo de conexión
    int32_t rta_handshake = 0;

    if(recv(socket_interfaz, &tipo_handshake, sizeof(tipo_handshake), MSG_WAITALL) < 0) {
        perror("Error en handshake");
        close(socket_interfaz);
        return ERROR;
    }
    // tipo_handshake deberia leer un enum distinto dependiendo del modulo interfaz que se conecto asi podemos identificarlo
    if(tipo_handshake < 0 || tipo_handshake >= HANDSHAKE_ERROR)
        rta_handshake = -1;

    ssize_t bytes_send = send(socket_interfaz, &rta_handshake, sizeof(rta_handshake), 0);

    if(bytes_send < 0 || rta_handshake != 0) {
        perror("Error en handshake");
        close(socket_interfaz);
        return ERROR;
    }
    // No salgo del hilo para no perder el socket_interfaz y manejo los comandos según el modulo conectado
    manejar_interfaz(tipo_handshake, socket_interfaz);

    return OK;
}

void* thread_handshake_con_interfaz(void* fd_interfaz) {
    int error = handshake_con_interfaz(*(int*)fd_interfaz);
    close(*(int*)fd_interfaz);
    free(fd_interfaz);
    if(error < 0)
        exit(ERROR);
    pthread_exit(NULL);
    return NULL;
}

// Dentro de cada case en vez de un printf se ejecutaria una función distinta dependiendo del tipo de handshake
void manejar_interfaz(conexion_t handshake, int socket_interfaz) {
    String nombre;
    switch (handshake) {
        case GENERIC_CON_KERNEL:
            nombre = recibir_nombre(socket_interfaz);
            interfaz_t *interfaz = malloc(sizeof(interfaz_t));
            interfaz->socket = socket_interfaz;
            interfaz->tipo = GENERIC;
            dictionary_put(interfaces, nombre, interfaz); // Guardo la conexión en un diccionario y uso el nombre de la interfaz como key
            log_debug(extra_logger, "Interfaz GENERICA conectada con el KERNEL [%s]", nombre);
            printf("\n> ");
            fflush(stdout);
            send_io_gen_sleep(socket_interfaz, 1000); // TODO: Aca no va esto, solo para testear por ahora
            break;
        case STDIN_CON_KERNEL:
            log_debug(extra_logger, "Interfaz STDIN conectada con el KERNEL");
            break;
        case STDOUT_CON_KERNEL:
            log_debug(extra_logger, "Interfaz STDOUT conectada con el KERNEL");
            break;
        case DIALFS_CON_KERNEL:
            log_debug(extra_logger, "Interfaz DIALFS conectada con el KERNEL");
            break;
        default:
            log_error(extra_logger, "Error al tratar de identificar el handshake!");
            exit(ERROR);
    }
    free(nombre);
}

String recibir_nombre(int socket) {

    paquete_t *paquete = recibir_paquete(socket);

    if(paquete == NULL)
        exit(EXIT_FAILURE);

    String nombre = payload_read_string(paquete->payload);

    payload_destroy(paquete->payload);
    liberar_paquete(paquete);

    return nombre;
}

void send_io_gen_sleep(int socket, uint32_t tiempo) {

    payload_t *payload = payload_create(sizeof(uint32_t));

    payload_add(payload, &tiempo, sizeof(uint32_t));

    paquete_t *paquete = crear_paquete(IO_GEN_SLEEP, payload);

    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);

    payload_destroy(payload);
    liberar_paquete(paquete);
}