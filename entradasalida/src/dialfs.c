#include "dialfs.h"

extern t_interfaz_config *interfaz_config;
extern t_log *logger;
extern t_log *extra_logger;

//extras
int fd_kernel, fd_memoria;

char* bufferBloques;
t_bitarray *bufferBitmap;

char* ruta_metadata = "/home/utnso/Templates";

int tamanio_bloques, cantidad_bloques;
size_t tamanio_bitmap;


void interfaz_dialFS(String nombre)
{
    tamanio_bloques = interfaz_config->block_size;
    cantidad_bloques = interfaz_config->block_count;
    
    log_info(extra_logger, "cantidad de bloques: %i, tamanio de bloques: %i", cantidad_bloques, tamanio_bloques);

    inicializar_bloques("bloques.dat");
    inicializar_bitmap("bitmap.dat");

    //fd_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, DIALFS_CON_KERNEL, extra_logger);
    //fd_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, DIALFS_CON_MEMORIA, extra_logger);
    
    fd_kernel = 3;
    fd_memoria = 2;
    
    //liberar_espacio_bitmap(0, cantidad_bloques * tamanio_bloques);

    while(1){
        dialfd_procesar_instrucciones(fd_kernel, fd_memoria);
    }

    mostrar_bitmap();
    //close(fd_kernel);
    //close(fd_memoria);

}

void dialfd_procesar_instrucciones(int fd_kernel, int fd_memoria)
{
    /*
    uint16_t pid;

    paquete_t *paquete = recibir_paquete(fd_kernel);
    
    if(paquete == NULL) {
        log_info(logger, "No se pudo recibir el paquete de kernel");
        exit(EXIT_FAILURE);
    }
    
    payload_read(paquete->payload, &pid, sizeof(uint16_t));
    
    String nombre = payload_read_string(paquete->payload);
    */
    
    int pid = 1234;
    char* nombre = "prr3";
    int operacion = IO_FS_DELETE;    

    switch (operacion)//(paquete->operacion)
    {
    case IO_FS_CREATE:
        
        crear_archivo(pid, nombre);
        
        //payload_destroy(paquete->payload);
        //liberar_paquete(paquete);
        
        mostrar_bitmap();
        
        exit(EXIT_SUCCESS);
        
        break;

    case IO_FS_DELETE:
        
        eliminar_archivo(pid, nombre);
        
        // payload_destroy(paquete->payload);
        // liberar_paquete(paquete);
        
        mostrar_bitmap();
        exit(EXIT_SUCCESS);

        break;

    // case IO_FS_READ:
        
    //     payload_read(paquete->payload, &direccion_memoria, sizeof(int));
    //     payload_read(paquete->payload, &puntero_archivo, sizeof(int));        
    //     payload_read(paquete->payload, &tamanio_a_leer, sizeof(int));

    //     leer_archivo(nombre, direccion_memoria, puntero_archivo, tamanio_a_leer);
        
    //     payload_destroy(paquete->payload);
    //     liberar_paquete(paquete);
        
    //     break;

    // case IO_FS_TRUNCATE:
    //     size_t tamanio;

    //     payload_read(paquete->payload, &tamanio, sizeof(size_t));
        
    //     truncar_archivo(nombre, tamanio);
        
    //     payload_destroy(paquete->payload);
    //     liberar_paquete(paquete);
        
    //     break;

    // case IO_FS_WRITE:

    //     payload_read(paquete->payload, &direccion_memoria, sizeof(int));
    //     payload_read(paquete->payload, &puntero_archivo, sizeof(int));        
    //     payload_read(paquete->payload, &tamanio_a_leer, sizeof(int));

    //     escribir_archivo(nombre, direccion_memoria, puntero_archivo, puntero_archivo);

    //     payload_destroy(paquete->payload);
    //     liberar_paquete(paquete);
        
    //     break;

    default:

        log_error(logger, "Instruccion invalida!");
        
        //payload_destroy(paquete->payload);
        //liberar_paquete(paquete);
        
        exit(EXIT_FAILURE);

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

void inicializar_bitmap(String ruta_bitmap)
	{
		int tamanioBitmap = (cantidad_bloques + 7) / 8;;

		int file_descriptor = open(ruta_bitmap, O_CREAT | O_RDWR, 0664);
		if (file_descriptor == -1)
		{
			printf("Error al abrir archivo de bitmap\n");
			
			return;
		}
		ftruncate(file_descriptor, tamanioBitmap);

		void *string_bitmap = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
		// MAPEO A memoria

		if (string_bitmap == MAP_FAILED)
		{
			printf("Error al asignar memoria para el bitmap\n");
			close(file_descriptor);
			return;
		}

		bufferBitmap = bitarray_create_with_mode(string_bitmap, tamanioBitmap, LSB_FIRST);
		if (bufferBitmap == NULL)
		{
			printf("Error al crear el bitarray\n");
			munmap(string_bitmap, tamanioBitmap);
			close(file_descriptor);
			return;
		}

		// for (int i = 0; i < tamanioBitmap; i++)
		// {
		// 	bitarray_clean_bit(bufferBitmap, i);
		// }

		msync(string_bitmap, tamanioBitmap, MS_SYNC);

		close(file_descriptor);

        log_info(logger, "Se monto en memoria el archivo bitmap.dat");

        printf("\n");
	}

// **** OPERACIONES ****

// crea un archivo con tamaño cero, y su metadata
void crear_archivo(uint16_t pid, String nombre_archivo){
    
    int bloque_inicial = buscar_bloque_libre();
    if(bloque_inicial == -1){
        log_info(logger, "No se pudo crear el archivo, no hay bloques vacios");
        exit(EXIT_FAILURE);
    }
    
    bitarray_set_bit(bufferBitmap, bloque_inicial);
	
    //abre el archivo en forma w para crearlo y lo cierra
	char* path = string_from_format("%s/%s.txt", ruta_metadata, nombre_archivo);
	FILE * nuevofcb = fopen(path,"w");
	fclose(nuevofcb);
	
	//lo abre como config para cargarle los datos principales
	t_config * nuevoArchivo= iniciar_config(path);
	config_set_value(nuevoArchivo, "BLOQUE_INICIAL", string_itoa(bloque_inicial));
	config_set_value(nuevoArchivo, "TAMANIO_ARCHIVO", string_itoa(0));
	config_save(nuevoArchivo);
	config_destroy(nuevoArchivo);
	free(path);
    log_info(logger, "PID: %i, Crear Archivo: %s", pid, nombre_archivo);
    return;
}

void eliminar_archivo(uint16_t pid, String nombre_archivo)
{   
    char *pathArchivo = string_from_format("%s/%s.txt", ruta_metadata, nombre_archivo);

    if(access(pathArchivo, F_OK) == -1 ){
        log_info(logger, "No existe le archivo");
        exit(EXIT_FAILURE);
    }
    
    t_config *valores = config_create(pathArchivo);
    
    int bloque_inicial = config_get_int_value(valores, "BLOQUE_INICIAL");
    
    int tamanio_archivo = config_get_int_value(valores, "TAMANIO_ARCHIVO");
    
    liberar_espacio_bitmap(bloque_inicial, tamanio_archivo);
    //limpiar_bloques(bloque_inicial, tamanio_archivo);

    if(remove(pathArchivo) != 0){
        log_info(logger, "No se puedo eliminar el archivo %s", nombre_archivo);
        
        config_destroy(valores);
        free(pathArchivo);
        exit(EXIT_FAILURE);
    }

    config_destroy(valores);
    free(pathArchivo);

    log_info(logger,"PID: %i, Eliminar Archivo: %s", pid, nombre_archivo);
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

// void escribir_archivo(String nombre_archivo, int direccion_memoria, int cant_caracteres, int puntero_archivo){
    
//     log_info(extra_logger, "Escribiendo Archivo");
//     int estado_operacion = 0;

//     char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);
    
//     t_config *valores = config_create(pathArchivo);
    
//     if(valores == NULL){
//         perror("No se pudo abrir el archivo.");
//         exit(EXIT_FAILURE);
//     }

//     int tamArchivo = config_get_int_value(valores, "TAMANIO_ARCHIVO");
//     int primer_bloque = config_get_int_value(valores, "BLOQUE_INICIAL");

//     config_destroy(valores);
    
//     // le pido los datos a memoria
//     payload_t *payload_a_enviar = payload_create(sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t));
//     char operacion = 'R';
//     payload_add(payload_a_enviar, &operacion, sizeof(char));
//     payload_add(payload_a_enviar, &direccion_memoria, sizeof(uint32_t));
//     payload_add(payload_a_enviar, &cant_caracteres, sizeof(uint32_t));

//     paquete_t *paquete_a_enviar = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload_a_enviar);

//     if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK) {
//         log_error(extra_logger, "No se pudo enviar el paquete a la memoria!");
//         exit(EXIT_FAILURE);
//     }

//     payload_destroy(payload_a_enviar);
//     liberar_paquete(paquete_a_enviar);

//     //recibo los datos de memoria
//     paquete_t *respuesta = recibir_paquete(fd_memoria);

//     if(respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
//         log_error(extra_logger, "Error al recibir respuesta de la memoria\n");
//         exit(EXIT_FAILURE);
//     }

//     // contiene el string a escribir
//     char *buffer_a_escribir = malloc(cant_caracteres);

//     payload_read(respuesta->payload, buffer_a_escribir, cant_caracteres);

//     payload_destroy(respuesta->payload);
//     liberar_paquete(respuesta);

//     //   ***ARCHIVO***
//     size_t tamanioString = strlen(buffer_a_escribir);
    
//     escribir_bloques(primer_bloque, tamArchivo, puntero_archivo, buffer_a_escribir);

//     log_info(logger, "Escribir Archivo: %s, Tamaño a Escribir: %i, Puntero Archivo: %i", nombre_archivo, cant_caracteres, puntero_archivo);
// }

// void leer_archivo(String nombre_archivo, int direccion_memoria, int puntero_archivo, int tamanio_a_leer) {
    
//     char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);
    
//     t_config *metadata_archivo = config_create(pathArchivo);

//     if (metadata_archivo == NULL) {
//         log_error(extra_logger, "No se puedo obtener la informacion del archivo: %s", nombre_archivo);
//         free(pathArchivo);
//         return;
//     }

//     int bloque_inicial = config_get_int_value(metadata_archivo, "BLOQUE_INICIAL");
//     int tamanio_archivo = config_get_int_value(metadata_archivo, "TAMANIO_ARCHIVO");

//     if (puntero_archivo >= tamanio_archivo) {
//         log_error(extra_logger, "El puntero de archivo está fuera del tamaño del archivo: %s", nombre_archivo);
//         config_destroy(metadata_archivo);
//         free(pathArchivo);
//         return;
//     }

//     size_t bytes_a_leer = tamanio_a_leer;
//     if (puntero_archivo + tamanio_a_leer > tamanio_archivo) {
//         bytes_a_leer = tamanio_archivo - puntero_archivo;
//     }

//     // le pido los datos a memoria
//     payload_t *payload_a_enviar = payload_create(sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t));
//     char operacion = 'R';
//     payload_add(payload_a_enviar, &operacion, sizeof(char));
//     payload_add(payload_a_enviar, &direccion_memoria, sizeof(uint32_t));
//     payload_add(payload_a_enviar, &bytes_a_leer, sizeof(uint32_t));

//     paquete_t *paquete_a_enviar = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload_a_enviar);

//     if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK) {
//         log_error(extra_logger, "No se pudo enviar el paquete a la memoria!");
//         config_destroy(metadata_archivo);
//         free(pathArchivo);
//         return;
//     }

//     payload_destroy(payload_a_enviar);
//     liberar_paquete(paquete_a_enviar);

//     // recibo los datos de memoria
//     paquete_t *respuesta = recibir_paquete(fd_memoria);

//     if (respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
//         log_error(extra_logger, "Error al recibir respuesta de la memoria");
//         config_destroy(metadata_archivo);
//         free(pathArchivo);
//         return;
//     }

//     char *buffer_stdout = malloc(bytes_a_leer + 1);

//     payload_read(respuesta->payload, buffer_stdout, bytes_a_leer);

//     buffer_stdout[bytes_a_leer] = '\0';

//     payload_destroy(respuesta->payload);
//     liberar_paquete(respuesta);

//     log_info(logger, "Leer Archivo: %s Tamaño a Leer: %zu Puntero Archivo: %d", nombre_archivo, bytes_a_leer, puntero_archivo);
//     printf("Contenido leído: %s\n", buffer_stdout);

//     free(buffer_stdout);
//     config_destroy(metadata_archivo);
//     free(pathArchivo);
// }

// void compactar() {

// }


// **** OPERACIONES AUXILIARES ****

// devuelve el primer bloque vacio dentro del bitmap
int buscar_bloque_libre() {
    
    int tam = bitarray_get_max_bit(bufferBitmap);

    for(int i = 0; i < tam; i++){
            bool pos = bitarray_test_bit(bufferBitmap, i);
            if(pos == false){
                return i;
            }
    }
    return -1;
}

// devuelve la posicion donde se ecuentra el inicio del bloque
int obtener_inicio_bloque(int numero_bloque) {
    
    if (numero_bloque < 0 || numero_bloque >= cantidad_bloques) {
        return -1;
    }

    return numero_bloque * tamanio_bloques;
}

// imprime por cosola el contenido de los bloques
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

// imprime por consola el bitmap
void mostrar_bitmap(t_bitarray *bitmap) {

    int tamanioPosta = bitarray_get_max_bit(bufferBitmap);
    
    for (int i = 0; i < tamanioPosta; i++)
		{
			bool valor_bit = bitarray_test_bit(bufferBitmap, i);
            printf("Valor del bit[%i]: %d\n", i, valor_bit);
		}
}

void liberar_espacio_bitmap(int bloque_inicial, int tamanio_archivo) {
    
    int cant_bloques = ceil(tamanio_archivo / tamanio_bloques) + bloque_inicial;
    
    if(cant_bloques == bloque_inicial){
        
        bitarray_clean_bit(bufferBitmap, bloque_inicial);
    }else{

        for(int i = bloque_inicial; i < cant_bloques; i++){

            bitarray_clean_bit(bufferBitmap, i);

        }
    }
}

//limpia un bloque especifico
void limpiar_bloque(String vector, int bloque){
    int offset = bloque * tamanio_bloques;
    
    for(int i = 0; i < tamanio_bloques; i++){
        vector[offset + i] = '\0';
    }
}

// Función para escribir un texto largo en múltiples bloques
void escribir_texto_en_bloques(int primer_bloque, int offset_bloque, const char* texto) {
    size_t longitud_texto = strlen(texto);
    int bloques_necesarios = (longitud_texto + tamanio_bloques - 1) / tamanio_bloques;

    if (primer_bloque + bloques_necesarios > cantidad_bloques) {
        printf("El texto es demasiado largo para los bloques disponibles.\n");
        return;
    }

    // Escribir fragmentos en los bloques
    for (int i = 0; i < bloques_necesarios; ++i) {
        
        int offset = (primer_bloque * tamanio_bloques + i * tamanio_bloques) + offset_bloque;
        
        int longitud_copia = (strlen(texto) - i * tamanio_bloques < tamanio_bloques) ? (strlen(texto) - i * tamanio_bloques) : tamanio_bloques;
        
        memcpy(&bufferBloques[offset], &texto[i * tamanio_bloques], longitud_copia);
    }

    printf("Texto guardado en bloques correctamente.\n");
}
