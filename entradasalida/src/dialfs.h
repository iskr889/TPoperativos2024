#ifndef DIALFS_H
#define DIALFS_H

#include "main.h"
#include <commons/bitarray.h>
#include <fcntl.h>
#include <sys/stat.h> // obtenres atributos del archivo
#include <sys/mman.h> // mmap
#include <dirent.h> // directorios
#include <math.h>



void interfaz_dialFS(String nombre);

void inicializar_bitmap(String ruta_bitmap);
void inicializar_bloques(String ruta_bloques);

void dialfd_procesar_instrucciones(int fd_kernel, int fd_memoria);

// funciones auxiliares
void mostrar_bloques(String vector, int elementos_totales, int block_size);
int buscar_lugar_vacio_bitmap();
void liberar_espacio_bitmap(int bloque_inicial, int tamanio);
void escribir_bloques(int primer_bloque, size_t tam_archivo, int puntero_archivo, char* texto);
int obtener_inicio_bloque(int numero_bloque);

// operaciones
void crear_archivo(String nombre_archivo);
void eliminar_archivo(String nombre);
void truncar_archivo(String nombre, size_t tamanio);
void escribir_archivo(String nombre_archivo, int direccion_memoria, int cant_caracteres, int puntero_archivo);
void leer_arcivo(String nombre_archivo, int direccion_memoria, int cant_caracteres, int puntero_archivo);

void compactar();

#endif
