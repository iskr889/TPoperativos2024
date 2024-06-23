#include "tlb.h"

extern t_cpu_config cpu_config;
extern t_log* logger;

t_list *tlb;
uint16_t temp_pid; // Guarda un pid para ser accedido por la función is_entry_match()
uint32_t temp_pagina; // Guarda un numero de pagina para ser accedido por la función is_entry_match()
tlb_entry_t *oldest_entry; // Guarda el entry mas viejo de la tlb

void init_TLB() {
    tlb = list_create();
    printf("TLB INICIADA\n");
}

static bool is_entry_match(void *entry) {
    tlb_entry_t *e = (tlb_entry_t *)entry;
    return e->pid == temp_pid && e->pagina == temp_pagina;
}

uint8_t buscarPaginaTLB(uint16_t pid, uint32_t pagina) {
    temp_pid = pid;
    temp_pagina = pagina;
    return list_any_satisfy(tlb, is_entry_match) ? TLB_HIT : TLB_MISS;
}

uint32_t obtenerMarcoTLB(uint16_t pid, uint32_t pagina) {
    temp_pid = pid;
    temp_pagina = pagina;
    tlb_entry_t *entry = list_find(tlb, is_entry_match);
    entry->last_access = time(NULL);
    return entry->marco;
}

static void* last_accessed(void *entry1, void *entry2) {
    return ((tlb_entry_t *)entry1)->last_access < ((tlb_entry_t *)entry2)->last_access ? entry1 : entry2;
}

static bool is_oldest_entry(void *entry) {  
    return entry == oldest_entry;
}

void agregarEntradaTLB(uint16_t pid, uint32_t pagina, uint32_t marco) {
    if (list_size(tlb) >= cpu_config.cantidad_entradas) {
        if (strcmp(cpu_config.algoritmo_tlb, "FIFO") == 0) {
            list_remove_and_destroy_element(tlb, 0, free);
        } else if (strcmp(cpu_config.algoritmo_tlb, "LRU") == 0) {
            oldest_entry = list_get_minimum(tlb, last_accessed);
            list_remove_and_destroy_by_condition(tlb, is_oldest_entry, free);
        }
    }

    tlb_entry_t *new_entry = malloc(sizeof(tlb_entry_t));
    new_entry->pid = pid;
    new_entry->pagina = pagina;
    new_entry->marco = marco;
    new_entry->last_access = time(NULL);
    list_add(tlb, new_entry);

}

void destruir_TLB() {
    list_destroy_and_destroy_elements(tlb, free);
    printf("TLB DESTROY\n");
}

static void imprimirEntradaTLB(void *entry_tlb) {
    tlb_entry_t *entry = (tlb_entry_t *)entry_tlb;
    printf("PID: %d, Pagina: %d, Marco: %d, Last Access: %ld\n",
           entry->pid, entry->pagina, entry->marco, entry->last_access);
}

void imprimirTLB() {
    printf("Contenido de la TLB:\n");
    list_iterate(tlb, imprimirEntradaTLB);
}