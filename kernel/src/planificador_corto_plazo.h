#ifndef PLANIFICADORCORTOPLAZO_H_
#define PLANIFICADORCORTOPLAZO_H_

#include "main.h"

void dispatch_handler();

void* dispatcher();

void* thread_hilo_quantum(void *arg);

#endif