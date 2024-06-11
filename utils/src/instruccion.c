#include "instruccion.h"

instruccion_t *crear_instruccion(tipo_instruccion_t tipo, registro_t reg1, registro_t reg2, uint32_t valor, const char* interfaz, const char* recurso, const char* nombreArchivo) {
    instruccion_t *instruccion = malloc(sizeof(instruccion_t));
    if (instruccion == NULL) {
        perror("Error al asignar memoria para instruccion_t");
        exit(EXIT_FAILURE);
    }

    instruccion->tipo = tipo;
    instruccion->registro1 = reg1;
    instruccion->registro2 = reg2;
    instruccion->valor = valor;

    if (interfaz != NULL) {
        instruccion->interfaz = strdup(interfaz);
        if (instruccion->interfaz == NULL) {
            perror("Error al duplicar la cadena interfaz");
            free(instruccion);
            exit(EXIT_FAILURE);
        }
    } else {
        instruccion->interfaz = NULL;
    }

    if (recurso != NULL) {
        instruccion->recurso = strdup(recurso);
        if (instruccion->recurso == NULL) {
            perror("Error al duplicar la cadena recurso");
            free(instruccion->interfaz);
            free(instruccion);
            exit(EXIT_FAILURE);
        }
    } else {
        instruccion->recurso = NULL;
    }

    if (nombreArchivo != NULL) {
        instruccion->nombreArchivo = strdup(nombreArchivo);
        if (instruccion->nombreArchivo == NULL) {
            perror("Error al duplicar la cadena nombreArchivo");
            free(instruccion->recurso);
            free(instruccion->interfaz);
            free(instruccion);
            exit(EXIT_FAILURE);
        }
    } else {
        instruccion->nombreArchivo = NULL;
    }

    return instruccion;
}


payload_t *instruccion_serializar(instruccion_t *instruccion) {
    // Calcula el tamaño total necesario para serializar instruccion_t
    uint32_t interfaz_length = (instruccion->interfaz) ? strlen(instruccion->interfaz) + 1 : 0;
    uint32_t recurso_length = (instruccion->recurso) ? strlen(instruccion->recurso) + 1 : 0;
    uint32_t nombreArchivo_length = (instruccion->nombreArchivo) ? strlen(instruccion->nombreArchivo) + 1 : 0;

    uint32_t total_size = sizeof(tipo_instruccion_t) + 
                          sizeof(registro_t) * 2 + 
                          sizeof(uint32_t) + 
                          sizeof(uint32_t) + interfaz_length +
                          sizeof(uint32_t) + recurso_length +
                          sizeof(uint32_t) + nombreArchivo_length;

    payload_t *payload = payload_create(total_size);

    payload_add(payload, &instruccion->tipo, sizeof(tipo_instruccion_t));
    payload_add(payload, &instruccion->registro1, sizeof(registro_t));
    payload_add(payload, &instruccion->registro2, sizeof(registro_t));
    payload_add(payload, &instruccion->valor, sizeof(uint32_t));

    payload_add(payload, &interfaz_length, sizeof(uint32_t));
    if (interfaz_length > 0) {
        payload_add(payload, instruccion->interfaz, interfaz_length);
    }

    payload_add(payload, &recurso_length, sizeof(uint32_t));
    if (recurso_length > 0) {
        payload_add(payload, instruccion->recurso, recurso_length);
    }

    payload_add(payload, &nombreArchivo_length, sizeof(uint32_t));
    if (nombreArchivo_length > 0) {
        payload_add(payload, instruccion->nombreArchivo, nombreArchivo_length);
    }

    return payload;
}

instruccion_t *instruccion_deserializar(payload_t *payload) {
    instruccion_t *instruccion = malloc(sizeof(instruccion_t));

    payload->offset = 0;
    payload_read(payload, &instruccion->tipo, sizeof(tipo_instruccion_t));
    payload_read(payload, &instruccion->registro1, sizeof(registro_t));
    payload_read(payload, &instruccion->registro2, sizeof(registro_t));
    payload_read(payload, &instruccion->valor, sizeof(uint32_t));

    // Deserializar strings
    uint32_t length;
    payload_read(payload, &length, sizeof(uint32_t));
    instruccion->interfaz = malloc(length);
    payload_read(payload, instruccion->interfaz, length);

    payload_read(payload, &length, sizeof(uint32_t));
    instruccion->recurso = malloc(length);
    payload_read(payload, instruccion->recurso, length);

    payload_read(payload, &length, sizeof(uint32_t));
    instruccion->nombreArchivo = malloc(length);
    payload_read(payload, instruccion->nombreArchivo, length);

    return instruccion;
}


void enviar_instruccion(int socket, instruccion_t *instruccion) {
    payload_t *payload = instruccion_serializar(instruccion);
    paquete_t *paquete = crear_paquete(1, payload); // 1 es un ejemplo de código de operación
    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);
    payload_destroy(payload);
    liberar_paquete(paquete);
}

instruccion_t *recibir_instruccion(int socket) {
    paquete_t *paquete = recibir_paquete(socket);
    if(paquete == NULL)
        exit(EXIT_FAILURE);
    instruccion_t *instruccion = instruccion_deserializar(paquete->payload);
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
    return instruccion;
}


void imprimir_instruccion(instruccion_t *instruccion) {
    if (instruccion == NULL) {
        printf("La instrucción es NULL\n");
        return;
    }

    // Imprimir el tipo de instrucción
    switch (instruccion->tipo) {
        case I_SET:
            printf("Tipo: I_SET\n");
            break;
        case I_SUM:
            printf("Tipo: I_SUM\n");
            break;
        case I_SUB:
            printf("Tipo: I_SUB\n");
            break;
        case I_JNZ:
            printf("Tipo: I_JNZ\n");
            break;
        case I_IO_GEN_SLEEP:
            printf("Tipo: I_IO_GEN_SLEEP\n");
            break;
        case I_MOV_IN:
            printf("Tipo: I_MOV_IN\n");
            break;
        case I_MOV_OUT:
            printf("Tipo: I_MOV_OUT\n");
            break;
        case I_RESIZE:
            printf("Tipo: I_RESIZE\n");
            break;
        case I_COPY_STRING:
            printf("Tipo: I_COPY_STRING\n");
            break;
        case I_WAIT:
            printf("Tipo: I_WAIT\n");
            break;
        case I_SIGNAL:
            printf("Tipo: I_SIGNAL\n");
            break;
        case I_IO_STDIN_READ:
            printf("Tipo: I_IO_STDIN_READ\n");
            break;
        case I_IO_STDOUT_WRITE:
            printf("Tipo: I_IO_STDOUT_WRITE\n");
            break;
        case I_IO_FS_CREATE:
            printf("Tipo: I_IO_FS_CREATE\n");
            break;
        case I_IO_FS_DELETE:
            printf("Tipo: I_IO_FS_DELETE\n");
            break;
        case I_IO_FS_TRUNCATE:
            printf("Tipo: I_IO_FS_TRUNCATE\n");
            break;
        case I_IO_FS_WRITE:
            printf("Tipo: I_IO_FS_WRITE\n");
            break;
        case I_IO_FS_READ:
            printf("Tipo: I_IO_FS_READ\n");
            break;
        case I_EXIT:
            printf("Tipo: I_EXIT\n");
            break;
        default:
            printf("Tipo: Desconocido\n");
            break;
    }

    // Imprimir registros y valor
    printf("Registro 1: %d\n", instruccion->registro1);
    printf("Registro 2: %d\n", instruccion->registro2);
    printf("Valor: %u\n", instruccion->valor);

    // Imprimir strings si no son NULL
    if (instruccion->interfaz != NULL) {
        printf("Interfaz: %s\n", instruccion->interfaz);
    } else {
        printf("Interfaz: NULL\n");
    }

    if (instruccion->recurso != NULL) {
        printf("Recurso: %s\n", instruccion->recurso);
    } else {
        printf("Recurso: NULL\n");
    }

    if (instruccion->nombreArchivo != NULL) {
        printf("Nombre Archivo: %s\n", instruccion->nombreArchivo);
    } else {
        printf("Nombre Archivo: NULL\n");
    }
}