#include "recursos.h"

t_dictionary *recursos = NULL;
t_dictionary *recursos_asignados = NULL;
extern t_kernel_config* kernel_config;
extern scheduler_t *scheduler;
pthread_mutex_t recursos_mutex;

void init_recursos() {
    recursos = dictionary_create();
    recursos_asignados = dictionary_create();
    pthread_mutex_init(&recursos_mutex, NULL);

    for (int i = 0; kernel_config->recursos[i]; i++) {

        int *instancia_recurso = malloc(sizeof(int));

        if (instancia_recurso == NULL) {
            perror("No se pudo asignar memoria");
            exit(EXIT_FAILURE);
        }

        *instancia_recurso = atoi(kernel_config->instancias_recursos[i]);
        dictionary_put(recursos, kernel_config->recursos[i], instancia_recurso);
    }
}

void inicializar_recursos_proceso(int pid) {
    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pid); 
    t_dictionary *diccionario_recurso_proceso = dictionary_create();
    inicializar_lista_recursos_asignados(diccionario_recurso_proceso);
    dictionary_put(recursos_asignados, str_pid, diccionario_recurso_proceso);
}

void inicializar_lista_recursos_asignados(t_dictionary *diccionario) {

    for (int i = 0; kernel_config->recursos[i]; i++) {

        int *instancia_recurso = malloc(sizeof(int));

        if (instancia_recurso == NULL) {
            perror("No se pudo asignar memoria");
            exit(EXIT_FAILURE);
        }

        *instancia_recurso = 0;
        dictionary_put(diccionario, kernel_config->recursos[i], instancia_recurso);
    }
}

void restar_recurso(t_dictionary *diccionario, char* recurso) {
    int *instancia = dictionary_get(diccionario, recurso);
    (*instancia)--;
}

void sumar_recurso(t_dictionary *diccionario, char* recurso) {
    int *instancia = dictionary_get(diccionario, recurso);
    (*instancia)++;
}

void sumar_recursos(t_dictionary *diccionario, char* recurso, int cantidad) {
    int *instancia = dictionary_get(diccionario, recurso);
    (*instancia) = (*instancia) + cantidad;
}

int obtenerInstancia(t_dictionary *diccionario, char *recurso) {
    int *instancia = dictionary_get(diccionario, recurso);
    return *instancia;
}

void imprimir_instancia_prueba(t_dictionary *diccionario, char* recurso) {
    int *instancia = dictionary_get(diccionario, recurso);
    printf("El valor es: %d", *instancia);
}

void asignar_recurso_a_proceso(int pid, char *recurso) {
    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pid); 
    t_dictionary *lista_recursos_asignados = dictionary_get(recursos_asignados, str_pid);
    if (lista_recursos_asignados == NULL) {
        lista_recursos_asignados = dictionary_create();
        inicializar_lista_recursos_asignados(lista_recursos_asignados);
        dictionary_put(recursos_asignados, str_pid, lista_recursos_asignados);
    }
    sumar_recurso(lista_recursos_asignados, recurso);
}

void liberar_recurso_de_proceso(int pid, char *recurso) {
    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pid); 
    t_dictionary *lista_recusos_asignados = dictionary_get(recursos_asignados, str_pid); //Obtengo diccionario de recursos asignados al proceso
    restar_recurso(lista_recusos_asignados, recurso);
}

void liberar_recursos_de_proceso(int pid) {
    char str_pid[8];
    snprintf(str_pid, sizeof(str_pid), "%d", pid); 
    t_dictionary *lista_recusos_asignados = dictionary_get(recursos_asignados, str_pid); //Obtengo diccionario de recursos asignados al proceso
    dictionary_iterator(lista_recusos_asignados, liberar_recurso_aux);
}

void liberar_recurso_aux(char *key, void *instancia) {
    int *instancia_recurso = (int*)instancia;
    pthread_mutex_lock(&recursos_mutex);
    sumar_recursos(recursos, key, *instancia_recurso);
    if ((obtenerInstancia(recursos, key) >= 0)) {
        cola_blocked_a_ready(key);
    }
    pthread_mutex_unlock(&recursos_mutex);
}

