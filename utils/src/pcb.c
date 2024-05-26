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

void enviar_pcb(t_paquete* paquete_pcb, int socket_cliente) {
    int size;
    void* a_enviar = serializar_paquete_pcb(paquete_pcb, &size);
    send(socket_cliente, a_enviar, size, 0);
    free(a_enviar);
    free(paquete_pcb->buffer->stream);
    free(paquete_pcb->buffer);
    free(paquete_pcb);
}

t_paquete* crear_paquete_pcb(pcb_t pcb) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PCB;
	paquete->buffer = crear_buffer_pcb(pcb);
	return paquete;
}

t_buffer* crear_buffer_pcb(pcb_t pcb) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint16_t) * 2 + sizeof(cpu_reg_t) + sizeof(estados_t);
	buffer->offset = 0;
	buffer->stream = malloc(buffer->size);
	memcpy(buffer->stream + buffer->offset, &pcb.pid, sizeof(uint16_t));
	buffer->offset += sizeof(uint16_t);
	memcpy(buffer->stream + buffer->offset, &pcb.quantum, sizeof(uint16_t));
	buffer->offset += sizeof(uint16_t);
	memcpy(buffer->stream + buffer->offset, &pcb.registros, sizeof(cpu_reg_t));
	buffer->offset += sizeof(cpu_reg_t);
	memcpy(buffer->stream + buffer->offset, &pcb.estado, sizeof(estados_t));
	buffer->offset += sizeof(estados_t);
	return buffer;
}

void* serializar_paquete_pcb(t_paquete* paquete, int* size) {
    *size = sizeof(uint8_t) + sizeof(uint32_t) + paquete->buffer->size;
    void* a_enviar = malloc(*size);
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
    offset += sizeof(uint8_t);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    return a_enviar;
}

pcb_t* deserializar_pcb(t_buffer* buffer) {
    pcb_t* pcb = malloc(sizeof(pcb_t));
    void* stream = buffer->stream;
    memcpy(&(pcb->pid), stream, sizeof(uint16_t));
    stream += sizeof(uint16_t);
    memcpy(&(pcb->quantum), stream, sizeof(uint16_t));
    stream += sizeof(uint16_t);
    memcpy(&(pcb->registros), stream, sizeof(cpu_reg_t));
    stream += sizeof(cpu_reg_t);
    memcpy(&(pcb->estado), stream, sizeof(estados_t));
    return pcb;
}

t_paquete* recibir_paquete(int socket_cliente) {
    t_paquete* paquete = malloc(sizeof(t_paquete));
    t_buffer* buffer = malloc(sizeof(t_buffer));
    recv(socket_cliente, &(paquete->codigo_operacion), sizeof(uint8_t), 0);
    recv(socket_cliente, &(buffer->size), sizeof(uint32_t), 0);
    buffer->stream = malloc(buffer->size);
    recv(socket_cliente, buffer->stream, buffer->size, 0);

    paquete->buffer = buffer;
    return paquete;
}

pcb_t* recibir_pcb(int socket_cliente) {
    t_paquete* paquete_pcb = recibir_paquete(socket_cliente);
    pcb_t* pcb = deserializar_pcb(paquete_pcb->buffer);
    free(paquete_pcb->buffer->stream);
    free(paquete_pcb->buffer);
    free(paquete_pcb);

    return pcb;
}

void imprimir_pcb(pcb_t* pcb) {
    printf("PID: %d\n", pcb->pid);
    printf("Quantum: %d\n", pcb->quantum);
    printf("Registros:\n");
    printf("  PC: %d\n", pcb->registros.pc);
    printf("  AX: %d, BX: %d, CX: %d, DX: %d\n", pcb->registros.ax, pcb->registros.bx, pcb->registros.cx, pcb->registros.dx);
    printf("  EAX: %d, EBX: %d, ECX: %d, EDX: %d\n", pcb->registros.eax, pcb->registros.ebx, pcb->registros.ecx, pcb->registros.edx);
    printf("  SI: %d, DI: %d\n", pcb->registros.si, pcb->registros.di);
    printf("Estado: %d\n", pcb->estado);
}

//ENVIAR PCB
    // pcb_t mi_pcb = {
    //     .pid = 1234,
    //     .quantum = 5,
    //     .registros = { .pc = 1000, .ax = 1, .bx = 2, .cx = 3, .dx = 4, .eax = 100, .ebx = 200, .ecx = 300, .edx = 400, .si = 500, .di = 600 },
    //     .estado = READY
    // };
    // t_paquete* paquete_pcb = crear_paquete_pcb(mi_pcb);
    // enviar_pcb(paquete_pcb, conexion_dispatch);

    // pcb_t* pcb_recibido = recibir_pcb(socket_kernel);
    // imprimir_pcb(pcb_recibido);
    // free(pcb_recibido);