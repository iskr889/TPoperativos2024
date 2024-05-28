#include "utils.h"
#include "serializacion.h"

typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT
} estados_t;

typedef struct {
    uint32_t pc;
    uint8_t ax;
    uint8_t bx;
    uint8_t cx;
    uint8_t dx;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t si;
    uint32_t di;
} cpu_reg_t;

typedef struct {
    uint16_t pid;        // ID del proceso
    uint16_t quantum;    // Tiempo para RR y VRR
    cpu_reg_t registros; // Registros de la CPU
    estados_t estado;    // Estado del proceso
} pcb_t;

pcb_t* crear_proceso(uint16_t pid, uint16_t quantum);

payload_t *pcb_serializar(pcb_t *pcb); // Liberar payload_t despues de terminar de usar

pcb_t *pcb_deserializar(payload_t *payload); // Liberar pcb_t despues de terminar de usar

void imprimir_pcb(pcb_t* pcb);

void send_pcb(int socket, pcb_t *pcb);

pcb_t *receive_pcb(int socket);