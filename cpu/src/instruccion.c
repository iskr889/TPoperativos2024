#include "instruccion.h"

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

instruccion_t* fetch(pcb_t* pcb) {
    uint32_t pc = pcb->registros.pc;
    instruccion_t* instruccion = pedir_instruccion(pc);
    pcb->registros.pc += 1;
    return instruccion;
}

instruccion_t* pedir_instruccion(uint32_t pc) {
    enviar_pc(pc);
    return recibir_instruccion(socket_memoria);
}

void enviar_pc(uint32_t pc) {
  send(socket_memoria, &pc, sizeof(uint32_t), 0);
}

void decode(instruccion_t* instruccion, pcb_t* pcb) {
    switch (instruccion->tipo) {
        case I_SET:
        case I_SUM:
        case I_SUB:
        case I_JNZ:
            // Estas instrucciones ya están decodificadas
            break;

        case I_MOV_IN:
        case I_MOV_OUT:
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
            fprintf(stderr, "[ERROR] Instrucción no reconocida en decode: %d\n", instruccion->tipo);
            break;
    }
}


void execute(instruccion_t* instruccion, pcb_t* pcb) {
    switch (instruccion->tipo) {
        case I_SET:
            actualizar_registro(&pcb->registros, instruccion->registro1, instruccion->valor, ASIGNACION);
            break;

        case I_SUM:
            {
                void* reg2 = obtener_registro(&pcb->registros, instruccion->registro2);
                if (reg2) {
                    uint32_t valor2 = (instruccion->registro2 <= DX) ? *((uint8_t*)reg2) : *((uint32_t*)reg2);
                    actualizar_registro(&pcb->registros, instruccion->registro1, valor2, SUMA);
                } else {
                    fprintf(stderr, "[ERROR] Registro no válido: %d\n", instruccion->registro2);
                }
            }
            break;

        case I_SUB:
            {
                void* reg2 = obtener_registro(&pcb->registros, instruccion->registro2);
                if (reg2) {
                    uint32_t valor2 = (instruccion->registro2 <= DX) ? *((uint8_t*)reg2) : *((uint32_t*)reg2);
                    actualizar_registro(&pcb->registros, instruccion->registro1, valor2, RESTA);
                } else {
                    fprintf(stderr, "[ERROR] Registro no válido: %d\n", instruccion->registro2);
                }
            }
            break;

        case I_JNZ:
            {
                void* reg1 = obtener_registro(&pcb->registros, instruccion->registro1);
                if (reg1) {
                    uint32_t valor1 = (instruccion->registro1 <= DX) ? *((uint8_t*)reg1) : *((uint32_t*)reg1);
                    if (valor1 != 0) {
                        pcb->registros.pc = instruccion->valor;
                    }
                } else {
                    fprintf(stderr, "[ERROR] Registro no válido: %d\n", instruccion->registro1);
                }
            }
            break;

        case I_IO_GEN_SLEEP:
            usleep(instruccion->valor * 1000);
            break;

        case I_MOV_IN:
        printf("I_MOV_IN\n");
            // {
            //     uint32_t valor_memoria = obtener_valor_memoria(instruccion->valor); // Función a implementar
            //     actualizar_registro(&pcb->registros, instruccion->registro1, valor_memoria, ASIGNACION);
            // }
            break;

        case I_MOV_OUT:
            // {
            //     void* reg1 = obtener_registro(&pcb->registros, instruccion->registro1);
            //     if (reg1) {
            //         uint32_t valor_reg = (instruccion->registro1 <= DX) ? *((uint8_t*)reg1) : *((uint32_t*)reg1);
            //         almacenar_en_memoria(instruccion->valor, valor_reg); // Función a implementar
            //     } else {
            //         fprintf(stderr, "[ERROR] Registro no válido: %d\n", instruccion->registro1);
            //     }
            // }
            printf("I_MOV_OUT\n");
            break;

        case I_RESIZE:
            //resize_recurso(instruccion->recurso, instruccion->valor);
            printf("I_RESIZE\n");
            break;

        case I_COPY_STRING:
            // {
            //     char* string = obtener_cadena_de_registro(&pcb->registros, instruccion->registro1); // Función a implementar
            //     copiar_a_registro(&pcb->registros, instruccion->registro2, string); // Función a implementar
            //     free(string);
            // }
            printf("I_COPY_STRING\n");
            break;

        case I_WAIT:
            //wait_recurso(instruccion->recurso);
            printf("I_WAIT\n");
            break;

        case I_SIGNAL:
            //signal_recurso(instruccion->recurso);
            printf("I_SIGNAL\n");
            break;

        case I_IO_STDIN_READ:
            // {
            //     uint32_t valor_stdin = leer_stdin(); // Función a implementar
            //     actualizar_registro(&pcb->registros, instruccion->registro1, valor_stdin, ASIGNACION);
            // }
            printf("I_IO_STDIN_READ\n");
            break;

        case I_IO_STDOUT_WRITE:
            // {
            //     void* reg1 = obtener_registro(&pcb->registros, instruccion->registro1);
            //     if (reg1) {
            //         uint32_t valor_reg = (instruccion->registro1 <= DX) ? *((uint8_t*)reg1) : *((uint32_t*)reg1);
            //         escribir_stdout(valor_reg); // Función a implementar
            //     } else {
            //         fprintf(stderr, "[ERROR] Registro no válido: %d\n", instruccion->registro1);
            //     }
            // }
            printf("I_IO_STDOUT_WRITE\n");
            break;

        case I_IO_FS_CREATE:
        printf("I_IO_FS_CREATE\n");
            // Lógica de IO_FS_CREATE
            break;

        case I_IO_FS_DELETE:
        printf("I_IO_FS_DELETE\n");
            // Lógica de IO_FS_DELETE
            break;

        case I_IO_FS_TRUNCATE:
        printf("I_IO_FS_TRUNCATE\n");
            // Lógica de IO_FS_TRUNCATE
            break;

        case I_IO_FS_WRITE:
        printf("I_IO_FS_WRITE\n");
            // Lógica de IO_FS_WRITE
            break;

        case I_IO_FS_READ:
        printf("I_IO_FS_READ\n");
            // Lógica de IO_FS_READ
            break;

        case I_EXIT:
        printf("I_EXIT\n");
            // Finaliza el proceso
            pcb->estado = EXIT;
            break;

        default:
            // Manejo de instrucciones no reconocidas
            fprintf(stderr, "[ERROR] Instrucción no reconocida: %d\n", instruccion->tipo);
            break;
    }
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
        default: return NULL; // Manejo de error si el registro no es válido
    }
}



// uint32_t obtener_valor_memoria(uint32_t direccion) {
//     // Implementa la lógica para obtener el valor desde la memoria
// }

// void almacenar_en_memoria(uint32_t direccion, uint32_t valor) {
//     // Implementa la lógica para almacenar el valor en la memoria
// }

// void resize_recurso(char* recurso, uint32_t nuevo_tamano) {
//     // Implementa la lógica para redimensionar un recurso
// }

// char* obtener_cadena_de_registro(cpu_reg_t* registros, registro_t registro) {
//     // Implementa la lógica para obtener una cadena desde un registro
// }

// void copiar_a_registro(cpu_reg_t* registros, registro_t registro, char* string) {
//     // Implementa la lógica para copiar una cadena a un registro
// }

// void wait_recurso(char* recurso) {
//     // Implementa la lógica para wait (sincronización)
// }

// void signal_recurso(char* recurso) {
//     // Implementa la lógica para signal (sincronización)
// }

// uint32_t leer_stdin() {
//     // Implementa la lógica para leer desde stdin
// }

// void escribir_stdout
