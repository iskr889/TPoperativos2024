#include "instruccion.h"

extern int conexion_memoria;
extern int tam_pagina;
extern int conexion_dispatch;

void actualizar_registro(cpu_reg_t* registros, registro_t registro, uint32_t valor, operacion_t operacion) {
    void* reg = obtener_registro(registros, registro);
    if (reg) {
        if (registro <= DX) {
            uint8_t* reg8 = (uint8_t*)reg;
            switch (operacion) {
                case SUMA:
                    *reg8 += (uint8_t)valor;
                    break;
                case RESTA:
                    *reg8 -= (uint8_t)valor;
                    break;
                case ASIGNACION:
                    *reg8 = (uint8_t)valor;
                    break;
            }
        } else {
            uint32_t* reg32 = (uint32_t*)reg;
            switch (operacion) {
                case SUMA:
                    *reg32 += valor;
                    break;
                case RESTA:
                    *reg32 -= valor;
                    break;
                case ASIGNACION:
                    *reg32 = valor;
                    break;
            }
        }
    } else {
        fprintf(stderr, "[ERROR] Registro no válido: %d\n", registro);
    }
}

char* fetch(pcb_t* pcb) {
    uint32_t pc = pcb->registros.pc;
    uint16_t pid = pcb->pid;
    solicitar_intruccion(conexion_memoria, pid, pc);
    char* instruccion = recibir_instruccion(conexion_memoria);
    pcb->registros.pc += 1;
    return instruccion;
}

void decode(char* instruccion, pcb_t* pcb) {
    char **tokens = split_string(instruccion);
    switch (obtener_tipo_instruccion(tokens[0])) {
        case I_SET:
        case I_SUM:
        case I_SUB:
        case I_JNZ:
            // Estas instrucciones ya están decodificadas
            break;

        case I_MOV_IN:
        case I_MOV_OUT:
            //traducir_direccion_logica(tokens[0], pcb->pid, conexion_memoria);
            // Decodificar direcciones de memoria o registros
            //decodificar_mov(instruccion, pcb); // Función a implementar si es necesario
            break;

        case I_RESIZE:
        case I_COPY_STRING:
        case I_WAIT:
        case I_SIGNAL:
        case I_IO_GEN_SLEEP:
        case I_IO_STDIN_READ:
        case I_IO_STDOUT_WRITE:
        case I_IO_FS_CREATE:
        case I_IO_FS_DELETE:
        case I_IO_FS_TRUNCATE:
        case I_IO_FS_WRITE:
        case I_IO_FS_READ:
        case I_EXIT:
            // Decodificación específica si es necesario
            break;

        default:
            fprintf(stderr, "[ERROR] Instrucción no reconocida en decode\n");
            break;
    }
    free(tokens);
}


void execute(char* instruccion, pcb_t* pcb) {
    char **tokens = split_string(instruccion);
    switch (obtener_tipo_instruccion(tokens[0])) {
        case I_SET:
            actualizar_registro(&pcb->registros, getTipoRegistro(tokens[1]), atoi(tokens[2]), ASIGNACION);
            break;
        case I_SUM:
            void* reg2 = obtener_registro(&pcb->registros, getTipoRegistro(tokens[2]));
            if (reg2) {
                uint32_t valor2 = (getTipoRegistro(tokens[2]) <= DX) ? *((uint8_t*)reg2) : *((uint32_t*)reg2);
                actualizar_registro(&pcb->registros, getTipoRegistro(tokens[1]), valor2, SUMA);
            } else
                fprintf(stderr, "[ERROR] Registro no válido: %d\n", getTipoRegistro(tokens[2]));
            break;
        case I_SUB:
            void* reg = obtener_registro(&pcb->registros, getTipoRegistro(tokens[2]));
            if (reg) {
                uint32_t valor2 = (getTipoRegistro(tokens[2]) <= DX) ? *((uint8_t*)reg) : *((uint32_t*)reg);
                actualizar_registro(&pcb->registros, getTipoRegistro(tokens[1]), valor2, RESTA);
            } else
                fprintf(stderr, "[ERROR] Registro no válido: %d\n", getTipoRegistro(tokens[2]));
            break;

        case I_JNZ:
            void* reg1 = obtener_registro(&pcb->registros, getTipoRegistro(tokens[1]));
            if (reg1) {
                uint32_t valor1 = (getTipoRegistro(tokens[1]) <= DX) ? *((uint8_t*)reg1) : *((uint32_t*)reg1);
                if (valor1 != 0) {
                    pcb->registros.pc = atoi(tokens[2]);
                }
            } else
                fprintf(stderr, "[ERROR] Registro no válido: %d\n", getTipoRegistro(tokens[1]));
            break;
        case I_MOV_IN:
            uint32_t direccion_logica = atoi(tokens[2]);
            uint32_t direccion_fisica = traducir_direccion_logica(direccion_logica, pcb->pid, conexion_memoria, tam_pagina);
            uint32_t valor_memoria;
            if (leer_memoria(direccion_fisica, &valor_memoria, sizeof(uint32_t))) {
                actualizar_registro(&pcb->registros, getTipoRegistro(tokens[1]), valor_memoria, ASIGNACION);
            } else {
                fprintf(stderr, "[ERROR] Error al leer memoria en la dirección física: %d\n", direccion_fisica);
            }
            break;

        case I_MOV_OUT:
            void* registro = obtener_registro(&pcb->registros, getTipoRegistro(tokens[1]));
            if (registro) {
                uint32_t valor_reg = (getTipoRegistro(tokens[1]) <= DX) ? *((uint8_t*)registro) : *((uint32_t*)registro);
                uint32_t direccion_logica = atoi(tokens[2]);
                uint32_t direccion_fisica = traducir_direccion_logica(direccion_logica, pcb->pid, conexion_memoria, tam_pagina);
                if (!escribir_memoria(direccion_fisica, &valor_reg, sizeof(uint32_t))) {
                    fprintf(stderr, "[ERROR] Error al escribir en memoria en la dirección física: %d\n", direccion_fisica);
                }
            } else {
                fprintf(stderr, "[ERROR] Registro no válido: %d\n", getTipoRegistro(tokens[1]));
            }
            break;
        case I_RESIZE:
            resize(pcb, atoi(tokens[1]));
            break;
        case I_COPY_STRING:
            break;
        case I_WAIT:
            enviar_interrupcion(conexion_dispatch, pcb, instruccion, I_WAIT);
            break;
        case I_SIGNAL:
            enviar_interrupcion(conexion_dispatch, pcb, instruccion, I_SIGNAL);
            break;
        case I_IO_GEN_SLEEP:
            enviar_interrupcion(conexion_dispatch, pcb, instruccion, I_IO_GEN_SLEEP);
            break;
        case I_IO_STDIN_READ:
            i_io_stdin_operation(tokens[0],tokens[1],tokens[2],tokens[3], I_IO_STDIN_READ, pcb);
            break;
        case I_IO_STDOUT_WRITE:
            i_io_stdin_operation(tokens[0],tokens[1],tokens[2],tokens[3], I_IO_STDOUT_WRITE, pcb);
            break;
        case I_IO_FS_CREATE:
            enviar_interrupcion(conexion_dispatch, pcb, instruccion, I_IO_FS_CREATE);
            break;
        case I_IO_FS_DELETE:
            enviar_interrupcion(conexion_dispatch, pcb, instruccion, I_IO_FS_DELETE);
            break;
        case I_IO_FS_TRUNCATE:
            uint32_t tamanio = *(uint32_t*)obtener_valor_registro(pcb,tokens[3]);
            size_t instruccion_len = snprintf(NULL, 0, "I_IO_FS_TRUNCATE %s %s %u",tokens[1], tokens[2], tamanio) + 1;
            char *instruccion = (char*)malloc(instruccion_len);
            snprintf(instruccion, instruccion_len, "I_IO_FS_TRUNCATE %s %s %u",tokens[1], tokens[2], tamanio);
            enviar_interrupcion(conexion_dispatch, pcb, instruccion, I_IO_FS_TRUNCATE);
            free(instruccion);
            break;

        case I_IO_FS_WRITE:
            i_io_fs_operation("I_IO_FS_WRITE",tokens[1],tokens[2],tokens[3],tokens[4],tokens[5], I_IO_FS_READ,pcb);
            break;
        case I_IO_FS_READ:
            i_io_fs_operation("I_IO_FS_READ",tokens[1],tokens[2],tokens[3],tokens[4],tokens[5], I_IO_FS_READ,pcb);
            break;
        case I_EXIT:
            printf("I_EXIT\n");
            pcb->estado = EXIT;
            break;

        default:
            // Manejo de instrucciones no reconocidas
            fprintf(stderr, "[ERROR] Instrucción no reconocida\n");
            break;
    }
}

void* obtener_valor_registro(pcb_t *pcb, char* registro) {
    uint16_t t_registro = getTipoRegistro(registro);
    switch(t_registro) {
        case PC: return &(pcb->registros.pc);
        case AX: return &(pcb->registros.ax);
        case BX: return &(pcb->registros.bx);
        case CX: return &(pcb->registros.cx);
        case DX: return &(pcb->registros.dx);
        case EAX: return &(pcb->registros.eax);
        case EBX: return &(pcb->registros.ebx);
        case ECX: return &(pcb->registros.ecx);
        case EDX: return &(pcb->registros.edx);
        case SI: return &(pcb->registros.si);
        case DI: return &(pcb->registros.di);
        default:
            fprintf(stderr, "Registro no soportado: %s\n", registro);
            exit(EXIT_FAILURE);
    }
    return NULL;
}

void i_io_fs_operation(char *t_instruccion, char *interfaz, char *nombre_archivo, char *direccion, char *tamanio, char *puntero_archivo, uint16_t cod, pcb_t *pcb) {
    uint32_t direccion_logica = *(uint32_t*)obtener_valor_registro(pcb,direccion);
    uint32_t tam = *(uint32_t*)obtener_valor_registro(pcb,tamanio);
    uint32_t puntero_file = *(uint32_t*)obtener_valor_registro(pcb,puntero_archivo);
    uint32_t direccion_fisica = traducir_direccion_logica(direccion_logica, pcb->pid, conexion_memoria, tam_pagina);
    size_t instruccion_len = snprintf(NULL, 0, "%s %s %s %u %u %u",t_instruccion, interfaz, nombre_archivo, direccion_fisica, tam, puntero_file) + 1;
    char *instruccion = (char*)malloc(instruccion_len);
    snprintf(instruccion, instruccion_len, "%s %s %s %u %u %u",t_instruccion, interfaz, nombre_archivo, direccion_fisica, tam, puntero_file);
    enviar_interrupcion(conexion_dispatch, pcb, instruccion, cod);
    free(instruccion);
}

void i_io_stdin_operation(char *t_instruccion, char *interfaz, char *direccion, char *tamanio, uint16_t cod, pcb_t *pcb) {
    uint32_t direccion_logica = *(uint32_t*)obtener_valor_registro(pcb,direccion);
    uint32_t tam = *(uint32_t*)obtener_valor_registro(pcb,tamanio);
    uint32_t direccion_fisica = traducir_direccion_logica(direccion_logica, pcb->pid, conexion_memoria, tam_pagina);
    size_t instruccion_len = snprintf(NULL, 0, "%s %s %u %u",t_instruccion, interfaz, direccion_fisica, tam) + 1;
    char *instruccion = (char*)malloc(instruccion_len);
    snprintf(instruccion, instruccion_len, "%s %s %u %u",t_instruccion, interfaz, direccion_fisica, tam);
    enviar_interrupcion(conexion_dispatch, pcb, instruccion, cod);
    free(instruccion);
}

int getTipoRegistro(char *tipo) {
    if (strcmp(tipo, "AX") == 0) return AX;
    if (strcmp(tipo, "BX") == 0) return BX;
    if (strcmp(tipo, "CX") == 0) return CX;
    if (strcmp(tipo, "DX") == 0) return DX;
    if (strcmp(tipo, "EAX") == 0) return EAX;
    if (strcmp(tipo, "EBX") == 0) return EBX;
    if (strcmp(tipo, "ECX") == 0) return ECX;
    if (strcmp(tipo, "EDX") == 0) return EDX;
    if (strcmp(tipo, "SI") == 0) return SI;
    if (strcmp(tipo, "DI") == 0) return DI;
    return -1;
}

void* obtener_registro(cpu_reg_t* registros, registro_t registro) {
    switch (registro) {
        case AX: return &registros->ax;
        case BX: return &registros->bx;
        case CX: return &registros->cx;
        case DX: return &registros->dx;
        case EAX: return &registros->eax;
        case EBX: return &registros->ebx;
        case ECX: return &registros->ecx;
        case EDX: return &registros->edx;
        case SI: return &registros->si;
        case DI: return &registros->di;
        default: return NULL; // Manejo de error si el registro no es válido
    }
}

int obtener_tipo_instruccion(const char* tipo_str) {
    if (strcmp(tipo_str, "SET") == 0) return I_SET;
    if (strcmp(tipo_str, "SUM") == 0) return I_SUM;
    if (strcmp(tipo_str, "SUB") == 0) return I_SUB;
    if (strcmp(tipo_str, "JNZ") == 0) return I_JNZ;
    if (strcmp(tipo_str, "MOV_IN") == 0) return I_MOV_IN;
    if (strcmp(tipo_str, "MOV_OUT") == 0) return I_MOV_OUT;
    if (strcmp(tipo_str, "RESIZE") == 0) return I_RESIZE;
    if (strcmp(tipo_str, "COPY_STRING") == 0) return I_COPY_STRING;
    if (strcmp(tipo_str, "WAIT") == 0) return I_WAIT;
    if (strcmp(tipo_str, "SIGNAL") == 0) return I_SIGNAL;
    if (strcmp(tipo_str, "IO_GEN_SLEEP") == 0) return I_IO_GEN_SLEEP;
    if (strcmp(tipo_str, "IO_STDIN_READ") == 0) return I_IO_STDIN_READ;
    if (strcmp(tipo_str, "IO_STDOUT_WRITE") == 0) return I_IO_STDOUT_WRITE;
    if (strcmp(tipo_str, "IO_FS_CREATE") == 0) return I_IO_FS_CREATE;
    if (strcmp(tipo_str, "IO_FS_DELETE") == 0) return I_IO_FS_DELETE;
    if (strcmp(tipo_str, "IO_FS_TRUNCATE") == 0) return I_IO_FS_TRUNCATE;
    if (strcmp(tipo_str, "IO_FS_WRITE") == 0) return I_IO_FS_WRITE;
    if (strcmp(tipo_str, "IO_FS_READ") == 0) return I_IO_FS_READ;
    if (strcmp(tipo_str, "EXIT") == 0) return I_EXIT;
    return -1;
}

char** split_string(char* str) {
    int spaces = 0;
    char* temp = str;

    while (*temp) {
        if (*temp == ' ') spaces++;
        temp++;
    }

    char** result = malloc((spaces + 2) * sizeof(char*));
    if (!result) {
        fprintf(stderr, "Error en la asignación de memoria.\n");
        exit(EXIT_FAILURE);
    }

    char* str_copy = strdup(str);
    if (!str_copy) {
        fprintf(stderr, "Error en la asignación de memoria.\n");
        exit(EXIT_FAILURE);
    }

    int idx = 0;
    char* token = strtok(str_copy, " ");
    while (token) {
        result[idx++] = token;
        token = strtok(NULL, " ");
    }
    result[idx] = NULL;

    return result;
}

void solicitar_intruccion(int socket, uint16_t pid, uint32_t pc) {
    payload_t *payload = payload_create(sizeof(pid) + sizeof(pc));
    payload_add(payload, &pid, sizeof(uint16_t));
    payload_add(payload, &pc, sizeof(uint32_t));
    paquete_t *paquete = crear_paquete(MEMORY_PID_PSEUDOCODE, payload);
    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);
    payload_destroy(payload);
    liberar_paquete(paquete);
}

char *recibir_instruccion(int socket) {
    paquete_t *paquete = recibir_paquete(socket);
    if(paquete == NULL || paquete->operacion != MEMORY_PID_PSEUDOCODE)
        exit(EXIT_FAILURE);
    char *instruccion = payload_read_string(paquete->payload);
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
    return instruccion;
}


bool leer_memoria(uint32_t direccion_fisica, void* buffer, uint32_t size) {
    payload_t *payload = payload_create(sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t));
    char operacion = 'R';
    payload_add(payload, &operacion, sizeof(char));
    payload_add(payload, &direccion_fisica, sizeof(uint32_t));
    payload_add(payload, &size, sizeof(uint32_t));
    paquete_t *paquete = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload);
    if (enviar_paquete(conexion_memoria, paquete) != OK) {
        payload_destroy(payload);
        liberar_paquete(paquete);
        return false;
    }

    payload_destroy(payload);
    liberar_paquete(paquete);

    paquete_t *respuesta = recibir_paquete(conexion_memoria);
    if (respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
        liberar_paquete(respuesta);
        return false;
    }

    payload_read(respuesta->payload, buffer, size);

    payload_destroy(respuesta->payload);
    liberar_paquete(respuesta);

    return true;
}

bool escribir_memoria(uint32_t direccion_fisica, void* buffer, uint32_t size) {
    payload_t *payload = payload_create(sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t) + size);
    char operacion = 'W';
    payload_add(payload, &operacion, sizeof(char));
    payload_add(payload, &direccion_fisica, sizeof(uint32_t));
    payload_add(payload, &size, sizeof(uint32_t));
    payload_add(payload, buffer, size);
    paquete_t *paquete = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload);
    if (enviar_paquete(conexion_memoria, paquete) != OK) {
        payload_destroy(payload);
        liberar_paquete(paquete);
        return false;
    }

    payload_destroy(payload);
    liberar_paquete(paquete);

    paquete_t *respuesta = recibir_paquete(conexion_memoria);
    if (respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
        liberar_paquete(respuesta);
        return false;
    }

    payload_destroy(respuesta->payload);
    liberar_paquete(respuesta);

    return true;
}

void io_fs_write(pcb_t *pcb, char* path, char* data, uint32_t direccion_logica, uint32_t size) {
    uint32_t direccion_fisica = traducir_direccion_logica(direccion_logica, pcb->pid, conexion_memoria, tam_pagina);
    size_t instruccion_len = snprintf(NULL, 0, "IO_FS_WRITE %s %u %u %s", path, direccion_fisica, size, data) + 1;
    char *instruccion = malloc(instruccion_len);
    snprintf(instruccion, instruccion_len, "IO_FS_WRITE %s %u %u %s", path, direccion_fisica, size, data);

    enviar_interrupcion(conexion_dispatch, pcb, instruccion, I_IO_FS_WRITE);
    free(instruccion);
}



void resize(pcb_t *pcb, uint32_t nuevo_tamano) {
    int resultado = solicitar_resize_memoria(pcb->pid, nuevo_tamano);

    if (resultado == OUT_OF_MEMORY) {
        size_t instruccion_len = snprintf(NULL, 0, "RESIZE OUT_OF_MEMORY") + 1;
        char *instruccion = malloc(instruccion_len);
        snprintf(instruccion, instruccion_len, "RESIZE OUT_OF_MEMORY");

        enviar_interrupcion(conexion_dispatch, pcb, instruccion, I_RESIZE);
        free(instruccion);
    }
}

int solicitar_resize_memoria(uint16_t pid, uint32_t nuevo_tamano) {
    payload_t *payload = payload_create(sizeof(pid) + sizeof(nuevo_tamano));
    payload_add(payload, &pid, sizeof(uint16_t));
    payload_add(payload, &nuevo_tamano, sizeof(uint32_t));

    paquete_t *paquete = crear_paquete(IO, payload);
    enviar_paquete(conexion_memoria, paquete);
    payload_destroy(payload);
    liberar_paquete(paquete);
    uint32_t resultado;
    paquete = recibir_paquete(conexion_memoria);
    payload_read(paquete->payload, &resultado, sizeof(uint32_t));
    liberar_paquete(paquete);
    payload_destroy(payload);
    return resultado;
}