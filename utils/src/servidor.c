#include "servidor.h"

int iniciar_servidor(String puerto) {
    struct addrinfo hints, *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if(getaddrinfo(NULL, puerto, &hints, &server_info)) {
        perror("Error en getaddrinfo");
        freeaddrinfo(server_info);
        return EXIT_ERROR;
    }

    // Creamos el socket de escucha del servidor
    int socket_servidor = socket(server_info->ai_family,
                                 server_info->ai_socktype,
                                 server_info->ai_protocol);

    if(socket_servidor < 0) {
        perror("Error en socket()");
        freeaddrinfo(server_info);
        return EXIT_ERROR;
    }

    uint32_t enable = 1; // Habilita el reusar address en el socket
    if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(uint32_t)) < 0) {
        perror("Error en setsockopt()");
        return EXIT_ERROR;
    }

    // Asociamos el socket a un puerto
    if(bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen)) {
        perror("Error en bind()");
        freeaddrinfo(server_info);
        return EXIT_ERROR;
    }

    // Escuchamos las conexiones entrantes
    if(listen(socket_servidor, 1)) {
        perror("Error en listen()");
        freeaddrinfo(server_info);
        return EXIT_ERROR;
    }

    freeaddrinfo(server_info);

    return socket_servidor;
}

int aceptar_clientes(int socket_servidor) {

    while(1) {

        int* socket_cliente = malloc(sizeof(int));

        if (socket_cliente == NULL) {
            perror("Error en malloc()");
            return EXIT_ERROR;
        }

        *socket_cliente = accept(socket_servidor, NULL, NULL);

        if (*socket_cliente < 0) {
            perror("Error en accept()");
            free(socket_cliente); // Libera la memoria en caso de error
            continue;
        }

        pthread_t thread;

        if (pthread_create(&thread, NULL, thread_handshake_con_cliente, socket_cliente)) {
            perror("Error en pthread_create()");
            close(*socket_cliente); // Cierro el socket del cliente en caso de error
            free(socket_cliente); // Libera la memoria en caso de error
            continue;
        }

        pthread_detach(thread);
    }

    close(socket_servidor);

    return EXIT_OK;
}

// Este handshake se ejecuta en un hilo que es creado despues del accept() y es unico por cada modulo conectado
int handshake_con_cliente(int socket_cliente) {
    handshake_t tipo_handshake = HANDSHAKE_ERROR; // Esto es un enum que identifica el tipo de conexión
    int32_t rta_handshake = 0;

    if(recv(socket_cliente, &tipo_handshake, sizeof(handshake_t), MSG_WAITALL) < 0) {
        perror("Error en handshake");
        close(socket_cliente);
        return EXIT_ERROR;
    }
    // tipo_handshake deberia leer un enum distinto dependiendo del modulo cliente que se conecto asi podemos identificarlo
    if(tipo_handshake < 0 || tipo_handshake >= HANDSHAKE_ERROR)
        rta_handshake = -1;

    ssize_t bytes_send = send(socket_cliente, &rta_handshake, sizeof(int32_t), 0);

    if(bytes_send < 0 || rta_handshake != 0) {
        perror("Error en handshake");
        close(socket_cliente);
        return EXIT_ERROR;
    }
    // No salgo del hilo para no perder el socket_cliente y manejo los comandos según el modulo conectado
    manejar_comandos(tipo_handshake, socket_cliente);

    return EXIT_OK;
}

void* thread_handshake_con_cliente(void* fd_cliente) {
    int error = handshake_con_cliente(*(int*)fd_cliente);
    close(*(int*)fd_cliente);
    free(fd_cliente);
    if(error < 0)
        exit(EXIT_ERROR);
    pthread_exit(NULL);
    return NULL;
}

void *thread_aceptar_clientes(void *socket_servidor) {
    aceptar_clientes(*(int*)socket_servidor);
    free(socket_servidor);
    pthread_exit(NULL);
    return NULL;
}

int modulo_escucha_conexiones_de(String nombre_modulos, String puerto, t_log* logger) {

    int fd_servidor = iniciar_servidor(puerto);

    if (fd_servidor < 0) {
        log_error(logger, "NO SE PUDO INICIAR EL SERVIDOR PARA QUE OTROS MODULOS SE CONECTEN");
        exit(EXIT_FAILURE);
    }
    
    log_info(logger, "SERVIDOR INICIADO... ESPERANDO LA CONEXION DE %s", nombre_modulos);

    return fd_servidor;
}

void atender_conexiones_al_modulo(pthread_t *hilo, int fd_servidor) {
        
    int* fd_servidor_p = malloc(sizeof(int));

    if (fd_servidor_p == NULL) {
        perror("Error en malloc()");
        exit(EXIT_ERROR);
    }

    *fd_servidor_p = fd_servidor;

    if (pthread_create(hilo, NULL, thread_aceptar_clientes, fd_servidor_p) != 0) {
        perror("No se pudo crear el hilo para manejar interfaces");
        free(fd_servidor_p);
        exit(EXIT_FAILURE);
    }

    return;
}

// Dentro de cada case en vez de un printf se ejecutaria una función distinta dependiendo del tipo de handshake
void manejar_comandos(handshake_t handshake, int socket_cliente) {

    switch (handshake) {
        case CPU_CON_MEMORIA:
            puts("CPU conectada con la MEMORIA\n");
            break;
        case KERNEL_CON_MEMORIA:
            puts("KERNEL conectado con la MEMORIA\n");
            break;
        case GENERIC_CON_MEMORIA:
            puts("Interfaz GENERICA conectada con la MEMORIA\n");
            break;
        case STDIN_CON_MEMORIA:
            puts("Interfaz STDIN conectada con la MEMORIA\n");
            break;
        case STDOUT_CON_MEMORIA:
            puts("Interfaz STDOUT conectada con la MEMORIA\n");
            break;
        case DIALFS_CON_MEMORIA:
            puts("Interfaz DIALFS conectada con la MEMORIA\n");
            break;
        case KERNEL_CON_CPU_DISPATCH:
            puts("KERNEL conectado con la CPU DISPATCH\n");
            break;
        case KERNEL_CON_CPU_INTERRUPT:
            puts("KERNEL conectado con la CPU INTERRUPT\n");
            break;
        case GENERIC_CON_KERNEL:
            puts("Interfaz GENERICA conectada con el KERNEL\n");
            break;
        case STDIN_CON_KERNEL:
            puts("Interfaz STDIN conectada con el KERNEL\n");
            break;
        case STDOUT_CON_KERNEL:
            puts("Interfaz STDOUT conectada con el KERNEL\n");
            break;
        case DIALFS_CON_KERNEL:
            puts("Interfaz DIALFS conectada con el KERNEL\n");
            break;
        default:
            puts("Error al tratar de identificar el handshake!\n");
            exit(EXIT_ERROR);
    }
}