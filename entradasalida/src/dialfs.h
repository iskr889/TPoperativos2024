#ifndef DIALFS_H
#define DIALFS_H

#include "main.h"
#include <commons/bitarray.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <math.h>

void interfaz_dialFS(String nombre);
void dialfd_procesar_instrucciones();

void inicializar_bitmap(String ruta_bitmap);
void inicializar_bloques(String ruta_bloques);

// funciones auxiliares
char* pedir_a_memoria(uint16_t pid, uint32_t tamanio, uint32_t direccion);
bool enviar_a_memoria(uint16_t pid, uint32_t direccion, String texto);

int buscar_primer_bloque_libre();
int buscar_siguiente_bloque_libre();

void liberar_espacio_bitmap(int boque_inicial, int tamanio);
void limpiar_bloques(int boque_inicial, int tamanio);
void escribir_texto_en_bloques(char *puntero_a_bloques, int primer_bloque, int tamanio_archivo, int puntero_archivo, char* texto);
void mostra_archivo(int primer_bloque, int tamanio_archivo);
char* obtener_texto(int bloque_inicial, int tamanio, int puntero_archivo);
int buscar_espacio_bitmap(int tamanio);
int obtener_inicio_bloque(int numero_bloque);
int verificar_y_crear_directorio(String ruta);
int compactar(String archivo_truncar, int tamanio);

// operaciones
void crear_archivo(uint16_t pid, String nombre);
void eliminar_archivo(uint16_t pid, String nombre);
void truncar_archivo(uint16_t pid,String nombre, int tamanio);
void leer_archivo(uint16_t pid,String nombre_archivo, int direccion, int tamanio, int puntero_archivo);
void escribir_archivo(uint16_t pid,String nombre_archivo, int direccion, int tamanio, int puntero_archivo);

int compactar(String archivo_truncar, int tamanio);

#endif