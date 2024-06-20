#include "interrupcion.h"

payload_t *interrupcion_serializar(pcb_t *pcb, String instruccion){
    
    uint32_t instruccion_length = strlen(instruccion) + 1;

    uint32_t total_size = sizeof(uint16_t) * 2 +  // pid, quantum
                          sizeof(uint32_t) * 7 +  // registros de CPU: pc, eax, ebx, ecx, edx, si, di
                          sizeof(uint8_t)  * 4 +  // registros de CPU: ax, bx, cx, dx
                          sizeof(estados_t) +     // estado
                          sizeof(uint32_t) + instruccion_length;//Instruccion

    payload_t *payload = payload_create(total_size);

    //agrego primero pcb
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

    //agrego despues instruccion
    payload_add_string(payload, instruccion);

    return payload;
}


interrupcion_t *interrupcion_deserializar(payload_t *payload) {
    
    pcb_t *pcb = malloc(sizeof(pcb_t));
    interrupcion_t *interrupcion = malloc(sizeof(interrupcion_t));

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

    // Deserializar strings
    String instruccion = payload_read_string(payload);

    interrupcion->pcb = pcb;
    interrupcion->instruccion = instruccion;

    return interrupcion;
}


void enviar_interrupcion(int socket, pcb_t *pcb, String instruccion, int codigo_operacion) {
    payload_t *payload = interrupcion_serializar(pcb, instruccion);
    paquete_t *paquete = crear_paquete(codigo_operacion, payload); // 1 es un ejemplo de código de operación
    if(enviar_paquete(socket, paquete) != OK)
        exit(EXIT_FAILURE);
    payload_destroy(payload);
    liberar_paquete(paquete);
}

paquete_t *recibir_interrupcion(int socket) {
    paquete_t *paquete = recibir_paquete(socket);
    if(paquete == NULL)
        exit(EXIT_FAILURE);

    return paquete;
}