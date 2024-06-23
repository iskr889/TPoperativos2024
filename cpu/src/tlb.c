#include "tlb.h"
t_list *tlb;

void init_TLB() {
    tlb = list_create();
    printf("TLB INICIADA\n");
}

uint8_t buscarPaginaTLB(uint16_t pid, uint32_t pagina) {
    bool is_entry_match(void *entry) {
        tlb_entry_t *e = (tlb_entry_t *)entry;
        return e->pid == pid && e->pagina == pagina;
    }

    return list_any_satisfy(tlb, is_entry_match) ? TLB_HIT : TLB_MISS;
}

uint32_t obtenerMarcoTLB(uint16_t pid, uint32_t pagina) {
    bool is_entry_match(void *entry) {
        tlb_entry_t *e = (tlb_entry_t *)entry;
        return e->pid == pid && e->pagina == pagina;
    }

    tlb_entry_t *entry = (tlb_entry_t *)list_find(tlb, is_entry_match);
    entry->last_access = time(NULL);
    return entry->marco;
}

// Cambiar tipo de retorno de bool a void* para usar con list_get_minimum
static void* is_oldest(void *entry1, void *entry2) {
    return ((tlb_entry_t *)entry1)->last_access < ((tlb_entry_t *)entry2)->last_access ? entry1 : entry2;
}

void agregarEntradaTLB(uint16_t pid, uint32_t pagina, uint32_t marco) {
    if (list_size(tlb) >= cpu_config.cantidad_entradas) {
        if (strcmp(cpu_config.algoritmo_tlb, "FIFO") == 0) {
            list_remove_and_destroy_element(tlb, 0, free);
        } else if (strcmp(cpu_config.algoritmo_tlb, "LRU") == 0) {
            tlb_entry_t *oldest_entry = list_get_minimum(tlb, is_oldest);
            bool is_entry_to_remove(void *entry) {  
                return entry == oldest_entry;
            }
            list_remove_and_destroy_by_condition(tlb, is_entry_to_remove, free);
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



void imprimirEntradaTLB(tlb_entry_t *entry) {
    printf("PID: %d, Pagina: %d, Marco: %d, Last Access: %ld\n",
           entry->pid, entry->pagina, entry->marco, entry->last_access);
}

void imprimirTLB() {
    void imprimirEntrada(void *entry) {
        imprimirEntradaTLB((tlb_entry_t *)entry);
    }
    printf("Contenido de la TLB:\n");
    list_iterate(tlb, imprimirEntrada);
}