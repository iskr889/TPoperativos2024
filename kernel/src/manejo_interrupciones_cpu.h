#ifndef MANEJOINTERRUPCIONESCPU_H_
#define MANEJOINTERRUPCIONESCPU_H_


#include "main.h"


void *manejo_interrupciones_cpu();
void actualizar_quantum(pcb_t *pcb, int quantum);
void *thread_hilo_mock_IO(void *arg);

#endif