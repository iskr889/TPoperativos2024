#include "main.h"
t_cpu_config config;
t_log* logger;


int main(int argc, char* argv[]) {
    t_config* temp_config = iniciar_config("cpu.config");
    load_logger();
    log_info(logger, "Archivo de configuración cargado correctamente");
    load_cpu_config(temp_config);
    pthread_t thread_dispatch, thread_interrupt;
    pthread_create(&thread_dispatch, NULL, (void*)iniciar_dispatch, NULL);
    pthread_detach(thread_dispatch);
    pthread_create(&thread_interrupt, NULL, (void*)iniciar_interrupt, NULL);
    pthread_detach(thread_interrupt);
    pthread_exit(0);
    log_destroy(logger);
    config_destroy(temp_config);
    return OK;
}