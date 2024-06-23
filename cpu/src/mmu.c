#include "mmu.h"

uint32_t traducir_direccion_logica(uint32_t direccion_logica, uint16_t pid, int socket_memoria, uint32_t tam_pagina) {
    uint32_t pagina = floor(direccion_logica / tam_pagina);
    uint32_t desplazamiento = direccion_logica - pagina * tam_pagina;
    uint32_t marco;
    uint8_t respuesta;
    if(estaActivaTLB())
        respuesta = buscarPaginaTLB(pid, pagina);
    else
        respuesta = TLB_DESACTIVADA;

    switch (respuesta) {
        case TLB_HIT:
            marco = obtenerMarcoTLB(pid, pagina);
            break;
        case TLB_MISS:
            marco = solicitarMarco(socket_memoria, pagina, pid);
            agregarEntradaTLB(pid, pagina, marco);
            break;
        default:
            marco = solicitarMarco(socket_memoria, pagina, pid);
            break;
    }
    return marco * tam_pagina + desplazamiento;
}

uint16_t estaActivaTLB() {
    int entradas = cpu_config.cantidad_entradas;
    return entradas > 0;
}

uint32_t solicitarMarco(int socket_memoria, uint32_t pagina, uint16_t pid) {
    payload_t *payload = payload_create(sizeof(uint16_t) + sizeof(uint32_t));
    payload_add(payload, &pid, sizeof(uint16_t));
    payload_add(payload, &pagina, sizeof(uint32_t));
    paquete_t *paquete = crear_paquete(MEMORY_PAGE_TABLE_ACCESS, payload);
    if(enviar_paquete(socket_memoria, paquete) != OK)
        exit(EXIT_FAILURE);
    payload_destroy(payload);
    liberar_paquete(paquete);
    return recibirMarco(socket_memoria);
}

uint32_t recibirMarco(int socket_memoria) {
    paquete_t *paquete = recibir_paquete(socket_memoria);
    if(paquete == NULL || paquete->operacion != MEMORY_RESPONSE_OK)
        exit(EXIT_FAILURE);
    uint32_t marco;
    payload_read(paquete->payload, &marco, sizeof(uint32_t));
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
    return marco;
}

uint32_t obtenerTamPagina(int socket_memoria) {
    return recibirTamPagina(socket_memoria);    
}

uint32_t recibirTamPagina(int socket_memoria) {
    paquete_t *paquete = recibir_paquete(socket_memoria);
    if(paquete == NULL || paquete->operacion != TAM_PAGINA)
        exit(EXIT_FAILURE);

    uint32_t tamanio;
    payload_read(paquete->payload, &tamanio, sizeof(uint32_t));
    payload_destroy(paquete->payload);
    liberar_paquete(paquete);
    return tamanio;
}