#include "pcb.h"
#include "serializacion.h"

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

payload_t *pcb_serializar(pcb_t *pcb) {
    // Calcula el tamaño total necesario para serializar pcb_t
    uint32_t total_size = sizeof(uint16_t) * 2 +  // pid, quantum
                          sizeof(uint32_t) * 7 +  // registros de CPU: pc, eax, ebx, ecx, edx, si, di
                          sizeof(uint8_t)  * 4 +  // registros de CPU: ax, bx, cx, dx
                          sizeof(estados_t);      // estado

    payload_t *payload = payload_create(total_size);

    payload_add(payload, &pcb->pid, sizeof(uint16_t));
    payload_add(payload, &pcb->quantum, sizeof(uint16_t));
    payload_add(payload, &pcb->registros.pc, sizeof(uint32_t));
    payload_add(payload, &pcb->registros.ax, sizeof(uint8_t));
    payload_add(payload, &pcb->registros.bx, sizeof(uint8_t));
    payload_add(payload, &pcb->registros.cx, sizeof(uint8_t));
    payload_add(payload, &pcb->registros.dx, sizeof(uint8_t));
    payload_add(payload, &pcb->registros.eax, sizeof(uint32_t));
    payload_add(payload, &pcb->registros.ebx, sizeof(uint32_t));
    payload_add(payload, &pcb->registros.ecx, sizeof(uint32_t));
    payload_add(payload, &pcb->registros.edx, sizeof(uint32_t));
    payload_add(payload, &pcb->registros.si, sizeof(uint32_t));
    payload_add(payload, &pcb->registros.di, sizeof(uint32_t));
    payload_add(payload, &pcb->estado, sizeof(estados_t));

    return payload;
}

pcb_t *pcb_deserializar(payload_t *payload) {
    pcb_t *pcb = malloc(sizeof(pcb_t));

    payload->offset = 0;
    payload_read(payload, &pcb->pid, sizeof(uint16_t));
    payload_read(payload, &pcb->quantum, sizeof(uint16_t));
    payload_read(payload, &pcb->registros.pc, sizeof(uint32_t));
    payload_read(payload, &pcb->registros.ax, sizeof(uint8_t));
    payload_read(payload, &pcb->registros.bx, sizeof(uint8_t));
    payload_read(payload, &pcb->registros.cx, sizeof(uint8_t));
    payload_read(payload, &pcb->registros.dx, sizeof(uint8_t));
    payload_read(payload, &pcb->registros.eax, sizeof(uint32_t));
    payload_read(payload, &pcb->registros.ebx, sizeof(uint32_t));
    payload_read(payload, &pcb->registros.ecx, sizeof(uint32_t));
    payload_read(payload, &pcb->registros.edx, sizeof(uint32_t));
    payload_read(payload, &pcb->registros.si, sizeof(uint32_t));
    payload_read(payload, &pcb->registros.di, sizeof(uint32_t));
    payload_read(payload, &pcb->estado, sizeof(estados_t));

    return pcb;
}

void imprimir_pcb(pcb_t* pcb) {
    puts("Imprimiendo pcb...\n");
    printf("PID: %d\n", pcb->pid);
    printf("Quantum: %d\n", pcb->quantum);
    printf("Registros:\n");
    printf("\tPC: %d\n", pcb->registros.pc);
    printf("\tAX: %d, BX: %d, CX: %d, DX: %d\n", pcb->registros.ax, pcb->registros.bx, pcb->registros.cx, pcb->registros.dx);
    printf("\tEAX: %d, EBX: %d, ECX: %d, EDX: %d\n", pcb->registros.eax, pcb->registros.ebx, pcb->registros.ecx, pcb->registros.edx);
    printf("\tSI: %d, DI: %d\n", pcb->registros.si, pcb->registros.di);
    printf("Estado: %d\n", pcb->estado);
}

void send_pcb(int socket, pcb_t *pcb) {
    payload_t *payload = pcb_serializar(pcb);
    paquete_t *paquete = crear_paquete(1, payload); // 1 es un ejemplo de código de operación
    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);
    payload_destroy(payload);
    liberar_paquete(paquete);
}

pcb_t *receive_pcb(int socket) {
    paquete_t *paquete = recibir_paquete(socket);
    if(paquete == NULL)
        exit(EXIT_FAILURE);
    pcb_t *pcb = pcb_deserializar(paquete->payload);
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);

    imprimir_pcb(pcb);

    return pcb;
}
