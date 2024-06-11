#include "recursos.h"

t_dictionary *recursos = NULL;
extern t_kernel_config* kernel_config;

void init_recursos(){
    recursos = dictionary_create();

    int i = 0;
    while(kernel_config->recursos[i] != NULL){
        int *instancia_recurso = malloc(sizeof(int));
        if (instancia_recurso == NULL) {
            perror("No se pudo asignar memoria");
            exit(EXIT_FAILURE);
        }
        *instancia_recurso = atoi(kernel_config->instancias_recursos[i]);
        dictionary_put(recursos, kernel_config->recursos[i],instancia_recurso);
        
        i++;

    }

}


void restar_recurso(t_dictionary *diccionario, char* recurso){
    int *instancia = dictionary_get(diccionario, recurso);
    (*instancia)--;

}

void sumar_recurso(t_dictionary *diccionario, char* recurso){
    int *instancia = dictionary_get(diccionario, recurso);
    (*instancia)++;

}

int obtenerInstancia(t_dictionary *diccionario, char *recurso){
    int *instancia = (int*)dictionary_get(diccionario, recurso);
    return *instancia;
}

void imprimir_instancia_prueba(t_dictionary *diccionario, char* recurso){
    int *instancia = dictionary_get(diccionario, recurso);
    printf("El valor es: %d", *instancia);
}




