/*
#include "../../utils/src/utils.h"
#include "../../utils/src/pcb.h"


t_list* cola_ready;

pcb_t* pcb;

while(1){

    switch(algoritmo){
        case FIFO:

            pcb = list_remove(cola_ready,0);

            t_paquete* paquete = serializar_paquete(pcb);

            enviar_paquete(paquete, sockect_cpu_dispatch);


            eliminar_pcb(pcb);
            eliminar_paquete(paquete);

        break;

    }

}
*/