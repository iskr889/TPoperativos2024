#ifndef DIALFS_H
#define DIALFS_H

#include "main.h"
#include <commons/bitarray.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <math.h>


void interfaz_dialFS               (String nombre);
void dialfd_procesar_instrucciones (int fd_kernel, int fd_memoria);

void inicializar_bitmap            (String ruta_bitmap);
void inicializar_bloques           (String ruta_bloques);

// funciones auxiliares
void mostrar_bloques               (String vector, int elementos_totales, int block_size);
void mostrar_bitmap                ();
int buscar_bloque_libre            ();
void liberar_espacio_bitmap        (int bloque_inicial, int tamanio);
int obtener_inicio_bloque          (int numero_bloque);
int buscar_espacio_bitmap          (int tamanio);
void mostra_archivo                (int primer_bloque, int tamanio_archivo);
void escribir_texto_en_bloques     (int primer_bloque, int tamanio_archivo, int puntero_archivo, char *texto);
void limpiar_bloques               (int primer_bloque, int tamanio_limpiar);
char* obtener_texto                (int bloque_inicial, int puntero_archivo, int tam_archivo, int tamanio_leer);

// operaciones
void crear_archivo                 (uint16_t pid, String nombre);
void eliminar_archivo              (uint16_t pid, String nombre);
void truncar_archivo               (uint16_t pid,String nombre, size_t tamanio);
void escribir_archivo              (uint16_t pid,String nombre_archivo, int direccion_memoria, int cant_caracteres, int puntero_archivo);
void leer_arcivo                   (uint16_t pid,String nombre_archivo, int direccion_memoria, int cant_caracteres, int puntero_archivo);
 
void compactar                     ();

#endif