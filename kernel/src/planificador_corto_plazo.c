//#include "../../utils/src/pcb.h"
// #include "planificador_corto_plazo.h"
// #include "scheduler.h"

// extern t_kernel_config* kernel_config;
// extern int conexion_interrupt;
// extern int conexion_dispatch;
// extern scheduler_t* scheduler;


// pthread_t hilo_quantum;

// void* dispatcher(){
//     //Me creo una pcb para tener en la funcion
//     pcb_t* pcb;
//     //aca despachamos para el algoritmo correspondiente
//     if(strcmp(kernel_config->algoritmo_planificacion, "FIFO")==0){

//         while(1){ 
//             //wait(sem_dispatcher);

//             //mutex
//             pcb = queue_pop(scheduler->cola_ready);
//             //mutex
//             t_paquete* paquete_pcb = crear_paquete_pcb(pcb);
//             enviar_pcb(paquete_pcb, conexion_dispatch);

//             //mutex
//             scheduler->execute = pcb;
//             //mutex

//             //eliminar_paquete(paquete);

            
//             }
//     }
//     else if(strcmp(kernel_config->algoritmo_planificacion, "RR")==0){

//         while(1){ 
//             //wait(sem_dispatcher)
//             pthread_create(&hilo_quantum,NULL, thread_hilo_quantum, NULL);

//             //mutex
//             pcb = queue_pop(scheduler->cola_ready);
//             //mutex

//             t_paquete* paquete_pcb = crear_paquete_pcb(pcb);
//             enviar_pcb(paquete_pcb, conexion_dispatch);

//             //mutex
//             scheduler->execute = pcb;
//             //mutex
//             //eliminar_paquete(paquete);
            
//             }
//     }
//     else if(strcmp(kernel_config->algoritmo_planificacion, "VRR")==0){

//         while(1){ 
//             //A determinar en el otro checkpoint
            
//             }
//     }

        
// } 

// void* thread_hilo_quantum(){
//     usleep(1000*(kernel_config->quantum));
//     //enviar_mensaje("Desalojo_quantum",conexion_interrupt);
//     pthread_exit(NULL);

// }

// void liberar_pcb(pcb_t* pcb){
//     free(pcb->recursos);
//     free(pcb);
// }
