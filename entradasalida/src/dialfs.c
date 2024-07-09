#include "dialfs.h"

extern t_interfaz_config *interfaz_config;
extern t_log *logger;
extern t_log *extra_logger;

//extras
int fd_kernel, fd_memoria;

char* bufferBloques;
t_bitarray *bufferBitmap;

size_t tamanio_bitmap;
char* ruta_metadata = "/home/utnso/Templates";

int tamanio_bloques, cantidad_bloques;


void interfaz_dialFS(String nombre)
{
    tamanio_bloques = interfaz_config->block_size;
    cantidad_bloques = interfaz_config->block_count;
    log_info(extra_logger, "cantidad de bloques: %i, tamanio de bloques: %i", cantidad_bloques, tamanio_bloques);

    inicializar_bloques("bloques.dat");
    inicializar_bitmap("bitmap.dat");

    //int fd_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, DIALFS_CON_KERNEL, extra_logger);
    //int fd_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, DIALFS_CON_MEMORIA, extra_logger);
    
    fd_kernel = 10;
    fd_memoria = 5;

    //while(1){
        //dialfd_procesar_instrucciones(fd_kernel, fd_memoria);
    //}
    
    mostrar_bloques(bufferBloques, tamanio_bloques * cantidad_bloques, tamanio_bloques);
    int pos = obtener_inicio_bloque(3);
    log_info(extra_logger,"pos: %i", pos);

    log_info(extra_logger, "Fin de programa");

    //close(fd_kernel);
    //close(fd_memoria);

    exit(EXIT_SUCCESS);
}

void dialfd_procesar_instrucciones(int fd_kernel, int fd_memoria)
{

    // paquete_t *paquete = recibir_paquete(fd_kernel);
    // char *nombre = payload_read_string(paquete->payload);

    int operacion = IO_FS_CREATE;
    char *nombre = "archivoPrueba4.txt";

    switch (operacion)//(paquete->operacion)
    {
    case IO_FS_CREATE:
        crear_archivo(nombre);
        break;
    case IO_FS_DELETE:
        eliminar_archivo(nombre);
        break;
    case IO_FS_READ:
        leer_archivo(nombre, )
        break;
    case IO_FS_TRUNCATE:
        size_t tamanio = 10; 
        //payload_read(paquete->payload, &tamanio, sizeof(size_t));
        truncar_archivo(nombre, tamanio);
        break;
    case IO_FS_WRITE:
        escribir_archivo(nombre, int direccion_memoria, int cant_caracteres, int puntero_archivo)
        break;

    default:
        log_error(logger, "Numero de instruccion invalido!");
        return;
        break;
    }
}

void inicializar_bloques(String ruta_bloques){
    
    int fd_bloques = open(ruta_bloques, O_RDWR | O_CREAT, 0644);
    struct stat s;                                            
    fstat(fd_bloques, &s);
    size_t tamArchivo = s.st_size;

    int tamanioArchivo = tamanio_bloques * cantidad_bloques; // tamanio = tamBloque * cantBloq

    if (tamArchivo == 0){ // Asigno tamanio al archivo si lo creo (sino ya tiene tam)
        ftruncate(fd_bloques, tamanioArchivo);
    }

    bufferBloques = mmap(NULL, tamanioArchivo, PROT_WRITE | PROT_READ, MAP_SHARED, fd_bloques, 0);
    // mmap(NULL, tamanioArchivo, permisos ,MAP_SHARED,archivo , puntero(posicion))

    if(bufferBloques == MAP_FAILED){
        log_info(logger,"Error al mapear el archivo bloques.dat");
        
        close(fd_bloques);
        free(bufferBloques);

        exit(EXIT_FAILURE);
    }
    
    log_info(extra_logger, "Se monto en memoria el archivo bloques.dat");
    
    close(fd_bloques);
}

void inicializar_bitmap(String ruta_bitmap){
    
    tamanio_bitmap = interfaz_config->block_count / CHAR_BIT;
    
    int fd_bitmap = open(ruta_bitmap, O_RDWR | O_CREAT, 0644);

    struct stat s;                                             // te da las caracteristicas del archivo
    fstat(fd_bitmap, &s);
    size_t tamArchivo = s.st_size;

    if(tamArchivo == 0 ){
        
        log_info(extra_logger, "Se creo el bitarray de cero");

        ftruncate(fd_bitmap, tamanio_bitmap);
    }
    
    log_info(extra_logger, "Se monto en memoria el arhcivo bitmap.dat");

    void *archivoBitmap = mmap(NULL, tamanio_bitmap, PROT_WRITE | PROT_READ, MAP_SHARED, fd_bitmap, 0);
    
    if(archivoBitmap == MAP_FAILED){
        log_info(logger,"Error al mapear el archivo bitmap.dat");
        
        close(fd_bitmap);
        free(archivoBitmap);

        exit(EXIT_FAILURE);
    }

    bufferBitmap = bitarray_create_with_mode(archivoBitmap, tamanio_bitmap, LSB_FIRST);
    
    msync(archivoBitmap, tamanio_bitmap, MS_SYNC);
    
    close(fd_bitmap);
}

// **** OPERACIONES ****

//verificar
void crear_archivo(String nombre_archivo){
    log_info(extra_logger, "Creando archivo");
    
    int estado_operacion = 0;

    int numero = buscar_lugar_vacio_bitmap();
    
    if(numero == -1){
        log_info(logger, "Bloques ocupados");
        exit(EXIT_FAILURE);
    }
    
    bitarray_set_bit(bufferBitmap, numero);

    // ruta del archivo, tengo que usar PATH_BASE_DIALFS
    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);

    t_config *metadata_archivo = malloc(sizeof(t_config));
    metadata_archivo->path = string_duplicate(pathArchivo);
    metadata_archivo->properties = dictionary_create();

    char *numero_bloque_inicial = string_itoa(numero);

    dictionary_put(metadata_archivo->properties, "BLOQUE_INICIAL", numero_bloque_inicial);
    
    char* valor_inicial = "0";
    dictionary_put(metadata_archivo->properties, "TAMANIO_ARCHIVO", valor_inicial);

    config_save(metadata_archivo);
    
    config_destroy(metadata_archivo);
    
    free(pathArchivo);

    log_info(logger, "Crear Archivo: %s", nombre_archivo);
}

//verificar
void eliminar_archivo(String nombre_archivo)
{   
    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);
    t_config *valores = config_create(pathArchivo);
    int bloque_inicial = config_get_int_value(valores, "BLOQUE_INICIAL");
    int tamanio = config_get_int_value(valores, "TAMANIO_ARCHIVO");
    
    liberar_espacio_bitmap(bloque_inicial, tamanio);
    //liberar_espacio_bloque(bloque_inicial, tamanio);

    if(remove(pathArchivo) != 0){
        log_info(logger, "No se puedo eliminar el archivo %s", nombre_archivo);
        
        config_destroy(valores);
        free(pathArchivo);
        exit(EXIT_FAILURE);
    }

    config_destroy(valores);
    free(pathArchivo);

    log_info(logger,"Eliminar Archivo: %s", nombre_archivo);
}

void truncar_archivo(String nombre_archivo, size_t nuevo_tamanio) {
    
    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);
    
    t_config *metadata_archivo = config_create(pathArchivo);

    if (metadata_archivo == NULL) {
        log_error(extra_logger, "No se pudo abrir el archivo para truncar: %s", nombre_archivo);
        free(pathArchivo);
        return;
    }

    int bloque_inicial = config_get_int_value(metadata_archivo, "BLOQUE_INICIAL");
    int tamanio_actual = config_get_int_value(metadata_archivo, "TAMANIO_ARCHIVO");
    
    int bloques_requeridos = (nuevo_tamanio + interfaz_config->block_size - 1) / interfaz_config->block_size;
    int bloques_actuales = (tamanio_actual + interfaz_config->block_size - 1) / interfaz_config->block_size;

    if (nuevo_tamanio < tamanio_actual) {
        // Liberar bloques adicionales
        for (int i = bloques_requeridos; i < bloques_actuales; i++) {
            bitarray_clean_bit(bufferBitmap, bloque_inicial + i);
        }
    } else if (nuevo_tamanio > tamanio_actual) {
        // Asignar bloques adicionales
        for (int i = bloques_actuales; i < bloques_requeridos; i++) {
            int nuevo_bloque = buscar_lugar_vacio_bitmap();
            if (nuevo_bloque == -1) {
                log_error(extra_logger, "No hay suficiente espacio para truncar el archivo: %s", nombre_archivo);
                config_destroy(metadata_archivo);
                free(pathArchivo);
                return;
            }
            bitarray_set_bit(bufferBitmap, nuevo_bloque);
        }
    }

    char *nuevo_tamanio_str = string_itoa(nuevo_tamanio);
    dictionary_put(metadata_archivo->properties, "TAMANIO_ARCHIVO", nuevo_tamanio_str);

    config_save(metadata_archivo);
    config_destroy(metadata_archivo);
    free(pathArchivo);
    free(nuevo_tamanio_str);

    log_info(logger, "Truncar Archivo: %s Tamaño: %zu", nombre_archivo, nuevo_tamanio);
}

void escribir_archivo(String nombre_archivo, int direccion_memoria, int cant_caracteres, int puntero_archivo){
    
    log_info(extra_logger, "Escribiendo Archivo");
    int estado_operacion = 0;

    // consume una unidad de trabajo

    //VERIFICO QUE EL ARCHIVO QUE QUIERO ESCRIBIR EXISTA
    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);
    
    t_config *valores = config_create(pathArchivo);
    
    if(valores == NULL){
        perror("No se pudo abrir el archivo.");
        exit(EXIT_FAILURE);
    }

    int tamArchivo = config_get_int_value(valores, "TAMANIO_ARCHIVO");
    int primer_bloque = config_get_int_value(valores, "BLOQUE_INICIAL");

    config_destroy(valores);
    
    // le pido los datos a memoria
    payload_t *payload_a_enviar = payload_create(sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t));
    char operacion = 'R';
    payload_add(payload_a_enviar, &operacion, sizeof(char));
    payload_add(payload_a_enviar, &direccion_memoria, sizeof(uint32_t));
    payload_add(payload_a_enviar, &cant_caracteres, sizeof(uint32_t));

    paquete_t *paquete_a_enviar = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload_a_enviar);

    if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK) {
        log_error(extra_logger, "No se pudo enviar el paquete a la memoria!");
        exit(EXIT_FAILURE);
    }

    payload_destroy(payload_a_enviar);
    liberar_paquete(paquete_a_enviar);

    //recibo los datos de memoria
    paquete_t *respuesta = recibir_paquete(fd_memoria);

    if(respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
        log_error(extra_logger, "Error al recibir respuesta de la memoria\n");
        exit(EXIT_FAILURE);
    }

    // contiene el string a escribir
    char *buffer_a_escribir = malloc(cant_caracteres);

    payload_read(respuesta->payload, buffer_a_escribir, cant_caracteres);

    //buffer_a_escribir[cant_caracteres] = '\0';

    payload_destroy(respuesta->payload);
    liberar_paquete(respuesta);

    //   ***ARCHIVO***
    size_t tamanioString = strlen(buffer_a_escribir);

    // busco la posicion dentro del archivo y luego escribo a partir de la misma
    
    escribir_bloques(primer_bloque, tamArchivo, puntero_archivo, buffer_a_escribir);

    log_info(logger, "Escribir Archivo: %s, Tamaño a Escribir: %i, Puntero Archivo: %i", nombre_archivo, cant_caracteres, puntero_archivo);
}

void leer_archivo(String nombre_archivo, int direccion_memoria, int puntero_archivo, size_t tamanio_a_leer) {
    
    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);
    
    t_config *metadata_archivo = config_create(pathArchivo);

    if (metadata_archivo == NULL) {
        log_error(extra_logger, "No se puedo obtener la informacion del archivo: %s", nombre_archivo);
        free(pathArchivo);
        return;
    }

    int bloque_inicial = config_get_int_value(metadata_archivo, "BLOQUE_INICIAL");
    int tamanio_archivo = config_get_int_value(metadata_archivo, "TAMANIO_ARCHIVO");

    if (puntero_archivo >= tamanio_archivo) {
        log_error(extra_logger, "El puntero de archivo está fuera del tamaño del archivo: %s", nombre_archivo);
        config_destroy(metadata_archivo);
        free(pathArchivo);
        return;
    }

    size_t bytes_a_leer = tamanio_a_leer;
    if (puntero_archivo + tamanio_a_leer > tamanio_archivo) {
        bytes_a_leer = tamanio_archivo - puntero_archivo;
    }

    // le pido los datos a memoria
    payload_t *payload_a_enviar = payload_create(sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t));
    char operacion = 'R';
    payload_add(payload_a_enviar, &operacion, sizeof(char));
    payload_add(payload_a_enviar, &direccion_memoria, sizeof(uint32_t));
    payload_add(payload_a_enviar, &bytes_a_leer, sizeof(uint32_t));

    paquete_t *paquete_a_enviar = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload_a_enviar);

    if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK) {
        log_error(extra_logger, "No se pudo enviar el paquete a la memoria!");
        config_destroy(metadata_archivo);
        free(pathArchivo);
        return;
    }

    payload_destroy(payload_a_enviar);
    liberar_paquete(paquete_a_enviar);

    // recibo los datos de memoria
    paquete_t *respuesta = recibir_paquete(fd_memoria);

    if (respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
        log_error(extra_logger, "Error al recibir respuesta de la memoria");
        config_destroy(metadata_archivo);
        free(pathArchivo);
        return;
    }

    char *buffer_stdout = malloc(bytes_a_leer + 1);

    payload_read(respuesta->payload, buffer_stdout, bytes_a_leer);

    buffer_stdout[bytes_a_leer] = '\0';

    payload_destroy(respuesta->payload);
    liberar_paquete(respuesta);

    log_info(logger, "Leer Archivo: %s Tamaño a Leer: %zu Puntero Archivo: %d", nombre_archivo, bytes_a_leer, puntero_archivo);
    printf("Contenido leído: %s\n", buffer_stdout);

    free(buffer_stdout);
    config_destroy(metadata_archivo);
    free(pathArchivo);
}

void compactar() {

}


// **** OPERACIONES AUXILIARES ****

int obtener_inicio_bloque(int numero_bloque) {
    
    if (numero_bloque < 0 || numero_bloque >= cantidad_bloques) {
        // El número de bloque está fuera de rango
        return -1;
    }

    return numero_bloque * tamanio_bloques;
}


void mostrar_bloques(String vector, int elementos_totales, int block_size) {
    int bloque_actual = 0;

    for (int i = 0; i < elementos_totales; i++) {
        if (i % block_size == 0) {
            printf("bloque[%d]: ", bloque_actual);
        }

        if (vector[i] == '\0') {
            printf("vacio, ");
        } else {
            printf("%c ", vector[i]);
        }

        if ((i + 1) % block_size == 0) {
            printf("\n");
            bloque_actual++;
        }
    }
    printf("\n");
}

//verificar
int buscar_lugar_vacio_bitmap(){

    for (int i = 0; i<tamanio_bitmap ;i++){
        
        if(bitarray_test_bit(bufferBitmap,i) == false){
            return i;
        }
    }

    return -1;
    
}

//verificar
void liberar_espacio_bitmap(int bloque_inicial, int tamanio) {
    
    // tamanio esta en bytes

    int cant_bloques = tamanio / interfaz_config->block_size;
    for(int i = bloque_inicial; i < cant_bloques; i++){
        
        bitarray_clean_bit(bufferBitmap, i);

    }
}

// tam_archivo = bytes

void escribir_bloques(int primer_bloque, size_t tam_archivo, int puntero_archivo, char* texto){
    
    int bloques_necesarios = Ceil(strlen(texto) / tamanio_bloques);
    
    if(tam_archivo < strlen(texto)){
        log_error(logger, "El texto es mas grande que el arhcivo");
        exit(EXIT_FAILURE);
    }

    int bloque = obtener_inicio_bloque(primer_bloque);

    int posicion = bloque + puntero_archivo;

    for (int i = 0; i < bloques_necesarios; ++i) {
        
        int offset = posicion * tamanio_bloques + i * tamanio_bloques;
        
        int longitud_copia = (strlen(texto) - i * tamanio_bloques < tamanio_bloques) ? (strlen(texto) - i * tamanio_bloques) : tamanio_bloques;
        
        memcpy(&bufferBloques[offset], &palabra[i * tamanio_bloques], longitud_copia);
    }

    msync(bufferBloques, tamanio_bloques * cantidad_bloques, MS_SYNC);

}