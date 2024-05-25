//#include "../../utils/src/pcb.h"
#include "main.h"
/*
extern scheduler_t* scheduler;
extern pthread_t hilo_quantum;


void* manejo_interrupciones_cpu(){

    int interrupcion = recibir_interrupcion(conexion_cpu);
    pcb_t* pcb;
    char* interfaz_name;
    switch(interrupcion){
        case FINALIZADO:
        pthread_cancel(hilo_quantum);

        //mutex
        queue_push(scheduler->cola_exit, pcb);
        //mutex
        //grado_multiprogramacion ++;



        //signal(sem_dispatcher)
        break;

        case IO:
        
        pthread_cancel(hilo_quantum);
        //mutex
        encolar_pcb_bloqueado(interfaz_name,scheduler->colas_blocked,pcb);
        //mutex
        


        //signal(sem_dispatcher)
        break;

        case QUANTUM:

        break;
    }

}
*/

/*
void encolar_pcb_bloqueado(char* interfaz_name, t_dictionary *diccionario, pcb_t *pcb){

    //void *pcb = (void *)data;
    t_list *cola = dictionary_get(diccionario, interfaz_name);

    if(cola != NULL){
        queue_push(cola, pcb)
        return;
    }
    cola = queue_create();
    queue_push(cola, pcb);
    dictionary_put(diccionario, interfaz_name, cola);

}



pcb_t *desencolar_bloqueado(char* interfaz_name, t_dictionary *diccionario){
    t_queue *cola = dictionary_get(diccionario, interfaz_name);
    if (cola == NULL) {
        perror("Mensaje error");
        return NULL;
    }
    pcb_t* pcb = queue_pop(cola);
    return pcb;
}
*/