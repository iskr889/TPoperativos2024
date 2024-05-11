#ifndef CONSOLA_H
#define CONSOLA_H

#include "../../utils/src/utils.h"

#define BUFF_SIZE 64

void exec_script(const String script_name);
void init_process(const String process_name);
void kill_process(const String pid);
void resume_scheduling(const String arg);
void stop_scheduling(const String arg);
void list_processes(const String arg);

// Función para el manejo de comandos
void manejar_comando(const String command);

// Thread para la interfaz de usuario
void* thread_consola(void* arg);

// Iniciar el thread_consola
int consola_kernel();

#endif