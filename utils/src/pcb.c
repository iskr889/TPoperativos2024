#include "pcb.h"

// Crear un nuevo PCB
pcb_t* crear_proceso(uint16_t pid, uint16_t quantum) {
    pcb_t *proceso = malloc(sizeof(pcb_t));
    if (proceso == NULL) {
        perror("Error en malloc()");
        return NULL; // Fallo en la asignación de memoria
    }
    proceso->pid = pid;
    proceso->quantum = quantum;
    proceso->registros.pc  = 0;
    proceso->registros.ax  = 0; proceso->registros.bx  = 0; proceso->registros.cx  = 0; proceso->registros.dx  = 0;
    proceso->registros.eax = 0; proceso->registros.ebx = 0; proceso->registros.ecx = 0; proceso->registros.edx = 0;
    proceso->registros.si  = 0; proceso->registros.di = 0;
    proceso->estado = NEW; // El estado inicial del proceso es NEW
    return proceso;
}
