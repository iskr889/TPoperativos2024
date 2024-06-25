#include "memory_interface_handler.h"
#include "manejar_instrucciones.h"

extern t_log* logger;
extern t_log* extra_logger;
extern t_memoria_config *memoria_config;

void manejar_interfaces(int fd_servidor) {

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
    aceptar_interfaces(*(int*)socket_servidor);
    free(socket_servidor);
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

int manejar_interfaz(conexion_t handshake, int socket_interfaz) {

    switch (handshake) {
        case STDIN_CON_MEMORIA:
            puts("Interfaz STDIN conectada con la MEMORIA\n");
            manejar_instrucciones_stdin(socket_interfaz);
            break;
        case STDOUT_CON_MEMORIA:
            puts("Interfaz STDOUT conectada con la MEMORIA\n");
            manejar_instrucciones_stdout(socket_interfaz);
            break;
        case DIALFS_CON_MEMORIA:
            puts("Interfaz DIALFS conectada con la MEMORIA\n");
            manejar_instrucciones_dialfs(socket_interfaz);
            break;
        default:
            puts("Error al tratar de identificar el handshake!\n");
            return ERROR;
    }
    return OK;
}

void manejar_instrucciones_stdin(int socket_stdin) {

    while (1) {

        paquete_t *paquete = recibir_paquete(socket_stdin);

        if (paquete == NULL) {
            fprintf(stderr, "Error al recibir paquete de STDIN");
            exit(EXIT_FAILURE);
        }

        ESPERAR_X_MILISEGUNDOS(memoria_config->retardo_respuesta);

        if (paquete->operacion != MEMORY_USER_SPACE_ACCESS) {
            log_error(extra_logger, "Operacion recibida de STDIN invalida");
            payload_destroy(paquete->payload);
            liberar_paquete(paquete);
            exit(EXIT_FAILURE);
        }

        instruccion_userspace_access(paquete->payload, socket_stdin);

        payload_destroy(paquete->payload);
        liberar_paquete(paquete);
    }
}

void manejar_instrucciones_stdout(int socket_stdout) {

    while (1) {

        paquete_t *paquete = recibir_paquete(socket_stdout);

        if (paquete == NULL) {
            fprintf(stderr, "Error al recibir paquete de STDOUT");
            exit(EXIT_FAILURE);
        }

        ESPERAR_X_MILISEGUNDOS(memoria_config->retardo_respuesta);

        if (paquete->operacion != MEMORY_USER_SPACE_ACCESS) {
            log_error(extra_logger, "Operacion recibida de STDOUT invalida");
            payload_destroy(paquete->payload);
            liberar_paquete(paquete);
            exit(EXIT_FAILURE);
        }

        instruccion_userspace_access(paquete->payload, socket_stdout);

        payload_destroy(paquete->payload);
        liberar_paquete(paquete);
    }
}

void manejar_instrucciones_dialfs(int socket_dialfs) {

}
