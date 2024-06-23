#ifndef TLB_H
#define TLB_H

#include "main.h"
#include <math.h>

typedef struct {
    int pid;
    int pagina;
    int marco;
    time_t last_access;
} tlb_entry_t;

typedef enum{
    TLB_DESACTIVADA,
    TLB_HIT,
    TLB_MISS
} status_tlb_t;

void agregarEntradaTLB(uint16_t pid, uint32_t pagina, uint32_t marco);
uint8_t buscarPaginaTLB(uint16_t pid, uint32_t pagina);
uint32_t obtenerMarcoTLB(uint16_t pid, uint32_t pagina);
void agregarEntradaTLB(uint16_t pid, uint32_t pagina, uint32_t marco);
void actualizarTLB(uint16_t pid, uint32_t pagina, uint32_t marco);
void imprimirTLB();
void iniciar_TLB();
void destruir_TLB();

#endif