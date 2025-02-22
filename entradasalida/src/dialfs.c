#include "dialfs.h"

extern t_interfaz_config *interfaz_config;
extern t_log *logger;
extern t_log *extra_logger;

int fd_kernel, fd_memoria;

char* bufferBloques;
t_bitarray *bufferBitmap;

char* ruta_metadata;

int tamanio_bloques, cantidad_bloques;


void interfaz_dialFS(String nombre)
{
    tamanio_bloques = interfaz_config->block_size;
    cantidad_bloques = interfaz_config->block_count;

    ruta_metadata = interfaz_config->path_base_dialfs;

    if(verificar_y_crear_directorio(ruta_metadata) != 0){
        log_info(extra_logger, "No se pudo acceder al directorio base del FS");
        exit(EXIT_FAILURE);
    }
    
    log_info(extra_logger, "cantidad de bloques: %d, tamanio de bloques: %d", cantidad_bloques, tamanio_bloques);

    bufferBloques = inicializar_bloques("bloques.dat");
    bufferBitmap = inicializar_bitmap("bitmap.dat");

    fd_kernel = conectarse_a_modulo("KERNEL", interfaz_config->ip_kernel, interfaz_config->puerto_kernel, DIALFS_CON_KERNEL, extra_logger);
    enviar_nombre_interfaz(nombre, fd_kernel);
    fd_memoria = conectarse_a_modulo("MEMORIA", interfaz_config->ip_memoria, interfaz_config->puerto_memoria, DIALFS_CON_MEMORIA, extra_logger);

    while(1){
        dialfd_procesar_instrucciones();
    }

    close(fd_kernel);
    close(fd_memoria);

}

void dialfd_procesar_instrucciones()
{
    uint16_t pid = 0;
    int direccion = 0;
    int tamanio = 0;
    int puntero_archivo = 0;
    String nombre_archivo;

    paquete_t* paquete;

    while(1){
        
        //mostrar_bitmap(bufferBitmap); // testeo

        paquete = recibir_paquete(fd_kernel);

        if(paquete == NULL)
            exit(EXIT_FAILURE);

        payload_read(paquete->payload, &pid, sizeof(uint16_t));
        nombre_archivo = payload_read_string(paquete->payload);

        switch (paquete->operacion)
        {
        case IO_FS_CREATE:
            crear_archivo(pid, nombre_archivo);
            break;
        case IO_FS_DELETE:
            eliminar_archivo(pid, nombre_archivo);
            break;
        case IO_FS_READ:
            
            payload_read(paquete->payload, &direccion, sizeof(int));
            payload_read(paquete->payload, &tamanio, sizeof(int));
            payload_read(paquete->payload, &puntero_archivo, sizeof(int));

            leer_archivo(pid, nombre_archivo, direccion, tamanio, puntero_archivo);
            break;
        case IO_FS_WRITE:

            payload_read(paquete->payload, &direccion, sizeof(int));
            payload_read(paquete->payload, &tamanio, sizeof(int));
            payload_read(paquete->payload, &puntero_archivo, sizeof(int));
            
            escribir_archivo(pid, nombre_archivo, direccion, tamanio, puntero_archivo);
            break;
        case IO_FS_TRUNCATE:

            payload_read(paquete->payload, &tamanio, sizeof(int));

            if(truncar_archivo(pid, nombre_archivo, tamanio) == false){
                log_info(logger, "PID: %i, Fallo al querer truncar el archivo: %s, Tamaño: %i", pid, nombre_archivo, tamanio);
            }
            
            break;
        default:
            log_error(logger, "No se reconoce la operacion");
            break;
        }

        enviar_operacion(OK, fd_kernel);
        free(nombre_archivo);
        payload_destroy(paquete->payload);
        liberar_paquete(paquete);
    }
}

char* inicializar_bloques(String ruta_bloques)
{

    int fd_bloques = open(ruta_bloques, O_RDWR | O_CREAT, 0644);
    struct stat s;
    fstat(fd_bloques, &s);
    size_t tamArchivo = s.st_size;

    int tamanioArchivo = tamanio_bloques * cantidad_bloques; // tamanio = tamBloque * cantBloq

    if (tamArchivo == 0)
    { // Asigno tamanio al archivo si lo creo (sino ya tiene tam)
        ftruncate(fd_bloques, tamanioArchivo);
    }

    void* buffer_bloques = mmap(NULL, tamanioArchivo, PROT_WRITE | PROT_READ, MAP_SHARED, fd_bloques, 0);
    // mmap(NULL, tamanioArchivo, permisos ,MAP_SHARED,archivo , puntero(posicion))

    if (buffer_bloques == MAP_FAILED)
    {

        log_info(extra_logger, "Error al mapear el archivo bloques.dat");

        close(fd_bloques);
        free(buffer_bloques);

        exit(EXIT_FAILURE);
    }

    log_info(extra_logger, "Se monto en memoria el archivo bloques.dat");

    close(fd_bloques);
    return (char*) buffer_bloques;
}

t_bitarray* inicializar_bitmap(String ruta_bitmap)
{
    int tamanioBitmap = ceil((double)cantidad_bloques / 8);

    int file_descriptor = open(ruta_bitmap, O_CREAT | O_RDWR, 0664);
    if (file_descriptor == -1)
    {
        printf("Error al abrir archivo de bitmap\n");

        return NULL;
    }
    ftruncate(file_descriptor, tamanioBitmap);

    void *string_bitmap = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor, 0);
    // MAPEO A memoria

    if (string_bitmap == MAP_FAILED)
    {
        printf("Error al asignar memoria para el bitmap\n");
        close(file_descriptor);
        return NULL;
    }

    t_bitarray* buffer_bitmap = bitarray_create_with_mode(string_bitmap, tamanioBitmap, LSB_FIRST);
    
    if (buffer_bitmap == NULL)
    {
        printf("Error al crear el bitarray\n");
        munmap(string_bitmap, tamanioBitmap);
        close(file_descriptor);
        return NULL;
    }

    msync(string_bitmap, tamanioBitmap, MS_SYNC);

    close(file_descriptor);

    log_info(extra_logger, "Se monto en memoria el archivo bitmap.dat");

    return buffer_bitmap;
}

int verificar_y_crear_directorio(char *ruta) {
    struct stat st;

    if (stat(ruta, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            log_info(extra_logger, "El directorio %s ya existe.", ruta);
            return 0;
        } else {
            log_info(extra_logger, "%s es un archivo, no un directorio.", ruta);
            return -1;
        }
    } else {

        if (mkdir(ruta, 0777) == 0) {
            log_info(extra_logger, "Directorio %s creado exitosamente.", ruta);
            return 0;
        } else {
            perror("Error al crear el directorio");
            return -1;
        }
    }
}

// **** OPERACIONES ****
void crear_archivo(uint16_t pid, String nombre_archivo)
{
    char *path = string_from_format("%s/%s", ruta_metadata, nombre_archivo);
    
    if(access(path, F_OK) == 0){
        printf("El archivo ya existe\n");
        free(path);
        return;
    }
    
    int bloque_inicial = buscar_primer_bloque_libre();
    
    if (bloque_inicial == -1)
    {
        printf("No se pudo crear el archivo, no hay bloques vacios\n");
        return;
    }

    bitarray_set_bit(bufferBitmap, bloque_inicial);

    // abre el archivo en forma w para crearlo y lo cierra
    FILE *nuevofcb = fopen(path, "w");
    fclose(nuevofcb);

    //guardo los datos
    if(guardar_metadata(nombre_archivo, bloque_inicial, 0) == false){
        
        return;
    }
    
    free(path);

    log_info(logger, "PID: %d, Crear Archivo: %s", pid, nombre_archivo);
}

void eliminar_archivo(uint16_t pid, String nombre_archivo)
{

    int bloque_inicial, tamanio_archivo;

    if(obtener_metadata(nombre_archivo, &bloque_inicial, &tamanio_archivo) == false){
        
        return;
    }

    int bloques_actuales = ceil((double)tamanio_archivo / (double)tamanio_bloques);

    liberar_espacio_bitmap(bloque_inicial, bloques_actuales );
    
    limpiar_bloques(bloque_inicial, tamanio_archivo);
    
    char *path_archivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);

    if (remove(path_archivo) != 0){

        printf("No se puedo eliminar el archivo %s\n", nombre_archivo);

        free(path_archivo);

        return;
    }

    log_info(logger, "PID: %d, Eliminar Archivo: %s", pid, nombre_archivo);
    
    free(path_archivo);
}

void escribir_archivo(uint16_t pid, String nombre_archivo, int direccion, int tamanio, int puntero_archivo)
{

    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);

    if (access(pathArchivo, F_OK) == -1)
    {
        printf("No existe la ruta especificada\n");
        exit(EXIT_FAILURE);
    }

    t_config *config = config_create(pathArchivo);

    if (config == NULL)
    {
        perror("No se pudo abrir el archivo");
        exit(EXIT_FAILURE);
    }

    int tamanio_archivo = config_get_int_value(config, "TAMANIO_ARCHIVO");
    int primer_bloque = config_get_int_value(config, "BLOQUE_INICIAL");

    config_destroy(config);

    if (tamanio_archivo == 0)
    {
        printf("El archivo tiene tamanio 0, no se puede escribir en el\n");
        exit(EXIT_FAILURE);
    }

    // le pido los datos a memoria

    char *texto_a_escribir = pedir_a_memoria(pid, tamanio, direccion);
    if(texto_a_escribir == NULL){
        printf("No se puedo leer de memoria el texto");
        exit(EXIT_FAILURE);
    }

    //   ***ARCHIVO***
    escribir_texto_en_bloques(bufferBloques, primer_bloque, tamanio_archivo, puntero_archivo, texto_a_escribir);

    free(texto_a_escribir);
    free(pathArchivo);

    mostra_archivo(primer_bloque, tamanio_archivo); // prueba

    log_info(logger, "PID: %d, Escribir Archivo: %s, Tamaño a Escribir: %d, Puntero Archivo: %d", pid, nombre_archivo, tamanio, puntero_archivo);
}

void leer_archivo(uint16_t pid, String nombre_archivo, int direccion, int tamanio, int puntero_archivo) {

    int bloque_inicial, tamanio_archivo;

    if(obtener_metadata(nombre_archivo, &bloque_inicial, &tamanio_archivo) == false){
        return;
    }

    if (puntero_archivo >= tamanio_archivo) {
        printf("El puntero está fuera del tamaño del archivo: %s\n", nombre_archivo);

        return;
    }

    // obtener el texto de los bloques
    char* texto_a_escribir = obtener_texto(bloque_inicial, tamanio, puntero_archivo);

    // envio el texto a memoria
    if(enviar_a_memoria(pid, direccion, texto_a_escribir) == false){
        printf("No se pudo escribir en memoria el texto");
        return;
    }
    
    log_info(logger, "PID: %d, Leer Archivo: %s Tamaño a Leer: %d Puntero Archivo: %d\n", pid, nombre_archivo, tamanio, puntero_archivo);

    free(texto_a_escribir);
}

int truncar_archivo(uint16_t pid, String nombre_archivo, int tamanio_nuevo) {

    int bloque_inicial, tamanio_actual;


    if(obtener_metadata(nombre_archivo, &bloque_inicial, &tamanio_actual) == false){
        
        return false;
    }

    int bloques_requeridos = ceil((double)tamanio_nuevo / (double)tamanio_bloques);
    int bloques_actuales = ceil((double)tamanio_actual / (double)tamanio_bloques);

    //libero para posiblemente reubicar el archivo
    liberar_espacio_bitmap(bloque_inicial, bloques_actuales);

    // si el archivo ya tiene el tamanio requerido
    if(tamanio_actual == tamanio_nuevo){
        
        printf("El archivo ya posee el tamaño.\n");

        printf("PID: %d, Truncar Archivo: %s Tamaño: %d\n", pid, nombre_archivo, tamanio_nuevo);
        
        return true;
    }

    // En caso de que el tamaño actual es suficiente actualizo solo la metadata
    if(tamanio_actual <= tamanio_bloques && tamanio_nuevo <= tamanio_bloques) {

        for(int i = bloque_inicial; i < bloque_inicial + bloques_requeridos; i++){
            bitarray_set_bit(bufferBitmap, i);
        }

        if(guardar_metadata(nombre_archivo, bloque_inicial, tamanio_nuevo) == false){
            
            return false;
        }

        printf("PID: %d, Truncar Archivo: %s Tamaño: %d\n", pid, nombre_archivo, tamanio_nuevo);

        return true;
    }

    // si el archivo lo tengo que hacer mas chico
    if(bloques_actuales >= bloques_requeridos){

        int bloques_eliminar = bloques_actuales - bloques_requeridos;
        
        int offset = bloque_inicial + bloques_requeridos;

        for(int i = bloque_inicial; i < bloque_inicial + bloques_requeridos; i++){
            bitarray_set_bit(bufferBitmap, i);
        }
        
        limpiar_bloques(offset, bloques_eliminar * tamanio_bloques);

        //asigno el nuevo TAMANIO_ARCHIVO
        if(guardar_metadata(nombre_archivo, bloque_inicial, tamanio_nuevo) == false){
            return false;
        }

        printf("PID: %d, Truncar Archivo: %s Tamaño: %d\n", pid, nombre_archivo, tamanio_nuevo);

        return true;
    }
    
    // si el archivo lo tengo que hacer mas grande
    if(bloques_actuales < bloques_requeridos){
        
        //libero para posiblemente reubicar el archivo
        //liberar_espacio_bitmap(bloque_inicial, bloques_actuales);

        //verifico que el espacio libre sea suficiente para el archivo a truncar
        if(comprobar_espacio_libre(bloques_requeridos) == false){
            
            //marco el espacio del archivo previamente liberado en el bitmap como ocupado
            for(int i = bloque_inicial; i < bloque_inicial + bloques_actuales; i++){
                bitarray_set_bit(bufferBitmap, i);
            }

            printf("No hay espacio suficiente para truncar el archivo: %s\n", nombre_archivo);
            return false;
        }

        int nuevo_bloque = buscar_espacio_bitmap(tamanio_nuevo);

        if(nuevo_bloque == -1){

            printf("No se pudo truncar el archivo debido a la falta de espacio libre contiguo\n");

            int compactar_resultado = compactar(pid, nombre_archivo, tamanio_nuevo);

            if(compactar_resultado == -1){
                
                //marco el espacio inicial dentro del bitmap
                for(int i = bloque_inicial; bloque_inicial + bloques_actuales; i++){
                    bitarray_set_bit(bufferBitmap, i);
                }

                return false;
            }
            
            printf("PID: %d, Truncar Archivo: %s Tamaño: %d\n", pid, nombre_archivo, tamanio_nuevo);

            return true;

        }

        // si se reposiciono el archivo dentro del bitmap
        if(bloque_inicial != nuevo_bloque){
            
            //mostrar_bitmap(bufferBitmap);

            //guado su contenido actual
            char* contenido_archivo = obtener_texto(bloque_inicial, tamanio_actual, 0);

            //tengo que borrar los datos
            limpiar_bloques(bloque_inicial, tamanio_actual);

            escribir_texto_en_bloques(bufferBloques, nuevo_bloque, tamanio_nuevo, 0, contenido_archivo);

            //tengo que reservar el espacio dentro del bitmap
            for(int i = nuevo_bloque; i < nuevo_bloque + bloques_requeridos; i++){
                bitarray_set_bit(bufferBitmap, i);
            }

            if(guardar_metadata(nombre_archivo, nuevo_bloque, tamanio_nuevo) == false){
                
                return false;
            }

            printf("PID: %d, Truncar Archivo: %s Tamaño: %d\n", pid, nombre_archivo, tamanio_nuevo);

            return true;

        }else{//si tiene el mismo bloque inicial

            // lo unico que tendria que hacer es marcar los nuevo bloques ocupados
            for(int i = bloque_inicial; i < bloque_inicial + bloques_requeridos; i++){
                bitarray_set_bit(bufferBitmap, i);
            }

            if(guardar_metadata(nombre_archivo, bloque_inicial, tamanio_nuevo) == false){

                return false;
            }

            printf("PID: %d, Truncar Archivo: %s Tamaño: %d\n", pid, nombre_archivo, tamanio_nuevo);

            return true;
        }

    }

    printf("Caso no reconocido en operacion truncar, bloques_actuales: %i, bloques_requeridos: %i\n", bloques_actuales, bloques_requeridos);
    
    return false;
}

// **** OPERACIONES AUXILIARES ****
char* pedir_a_memoria(uint16_t pid, uint32_t tamanio, uint32_t direccion)
{

    payload_t *payload_a_enviar = payload_create(sizeof(uint16_t) + sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t));

    char operacion = 'R'; // Necesario para leer memoria en espacio de usuario
    payload_add(payload_a_enviar, &pid, sizeof(uint16_t));
    payload_add(payload_a_enviar, &operacion, sizeof(char));
    payload_add(payload_a_enviar, &direccion, sizeof(uint32_t));
    payload_add(payload_a_enviar, &tamanio, sizeof(uint32_t));

    paquete_t *paquete_a_enviar = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload_a_enviar);

    if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK) {
        log_error(extra_logger, "No se pudo enviar el paquete a la memoria!");
        return false;
    }

    payload_destroy(payload_a_enviar);
    liberar_paquete(paquete_a_enviar);

    //RESPUESTA
    paquete_t *respuesta = recibir_paquete(fd_memoria);

    if(respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
        log_error(extra_logger, "Error al recibir respuesta de la memoria\n");
        return false;
    }

    char *buffer = malloc(tamanio);

    payload_read(respuesta->payload, buffer, tamanio);

    buffer[tamanio] = '\0';

    payload_destroy(respuesta->payload);
    liberar_paquete(respuesta);

    return buffer;

}

bool enviar_a_memoria(uint16_t pid, uint32_t direccion, String texto){
    
    int cant_caracteres = strlen(texto);

    payload_t *payload_a_enviar = payload_create(sizeof(uint16_t) + sizeof(char) + sizeof(uint32_t) + sizeof(uint32_t) + cant_caracteres);

    char operacion = 'W'; // Necesario para escribir memoria en espacio de usuario
    payload_add(payload_a_enviar, &pid, sizeof(uint16_t));
    payload_add(payload_a_enviar, &operacion, sizeof(char));
    payload_add(payload_a_enviar, &direccion, sizeof(uint32_t));
    payload_add(payload_a_enviar, &cant_caracteres, sizeof(uint32_t));
    payload_add(payload_a_enviar, texto, cant_caracteres);

    paquete_t *paquete_a_enviar = crear_paquete(MEMORY_USER_SPACE_ACCESS, payload_a_enviar);

    if (enviar_paquete(fd_memoria, paquete_a_enviar) != OK) {
        log_error(extra_logger, "No se pudo enviar el paquete a la memoria!");
        return false;
    }

    payload_destroy(payload_a_enviar);
    liberar_paquete(paquete_a_enviar);

    // Debajo recibo la respuesta de la memoria

    paquete_t *respuesta = recibir_paquete(fd_memoria);

    if(respuesta == NULL || respuesta->operacion != MEMORY_RESPONSE_OK) {
        log_error(extra_logger, "Error al recibir respuesta de la memoria\n");
        return false;
    }

    String rta_memoria = payload_read_string(respuesta->payload);

    log_info(extra_logger, "Memoria respondio: %s", rta_memoria);

    bool escritura_exitosa = strcmp(rta_memoria, "OK") == 0 ? true : false;

    free(rta_memoria);

    payload_destroy(respuesta->payload);
    liberar_paquete(respuesta);

    return escritura_exitosa;
}

int buscar_primer_bloque_libre()
{

    int tam = bitarray_get_max_bit(bufferBitmap);

    for (int i = 0; i < tam; i++)
    {
        bool pos = bitarray_test_bit(bufferBitmap, i);
        if (pos == false)
        {
            return i;
        }
    }
    return -1;
}

int buscar_siguiente_bloque_libre() {
    static int ultima_posicion = 0;
    int vuelta = 0;

    int tam = bitarray_get_max_bit(bufferBitmap);

    //realiza dos vueltas en el bitmap, para asegurar que busque todo
    while(vuelta < 2){
        
        for (int i = ultima_posicion; i < tam; i++) {
            bool pos = bitarray_test_bit(bufferBitmap, i);
            if (!pos) {
                ultima_posicion = i + 1;
                return i;
            }
        }
        
        if(ultima_posicion >= tam -1){
            ultima_posicion = 0;
        }

        vuelta++;
    }

    return -1;
}

void liberar_espacio_bitmap(int bloque_inicial, int cant_bloques)
{
    if(cant_bloques == 0){
        bitarray_clean_bit(bufferBitmap, bloque_inicial);
    }

    for (int i = bloque_inicial; i < cant_bloques + bloque_inicial; i++)
    {
        bitarray_clean_bit(bufferBitmap, i);
    }
}

void limpiar_bloques(int primer_bloque, int tamanio_limpiar)
{
    int inicio_archivo = obtener_inicio_bloque(primer_bloque);
    
    for (int i = inicio_archivo; i < inicio_archivo + tamanio_limpiar; i++)
    {
        memset(&bufferBloques[i], '\0', sizeof(char));
    }
}

void escribir_texto_en_bloques(char *puntero_a_bloques, int primer_bloque, int tamanio_archivo, int puntero_archivo, char *texto)
{
    size_t longitud_texto = strlen(texto);
    int bloques_disponibles = ceil((double)tamanio_archivo / (double)tamanio_bloques);
    int bloques_necesarios = ceil((double)longitud_texto / (double)tamanio_bloques);

    if (bloques_necesarios > bloques_disponibles)
    {
        printf("No hay espacio para escribir\n");
    }

    int bloque_inicial = obtener_inicio_bloque(primer_bloque);

    int offset = bloque_inicial + puntero_archivo;

    // int bloques = ceil((double)tamanio_archivo / (double)tamanio_bloques);
    // int total = primer_bloque + bloques;

    // // marco el bitmap
    // for (int i = primer_bloque; i < total; i++)
    // {
    //     bitarray_set_bit(bufferBitmap, i);
    //     // printf("Bloque[%d]: marcado como ocupado\n", i);
    // }

    memcpy(&puntero_a_bloques[offset], texto, longitud_texto);

    msync(puntero_a_bloques, tamanio_bloques * cantidad_bloques, MS_SYNC);

    //printf("Texto guardado en bloques correctamente.\n");
}

void mostra_archivo(int primer_bloque, int tamanio_archivo)
{
    int bloque_inicial = obtener_inicio_bloque(primer_bloque);

    printf("contenido del archivo: ");
    for (int i = bloque_inicial; i < tamanio_archivo + bloque_inicial; i++)
    {
        printf("%c", bufferBloques[i]);

        if (bufferBloques[i] == '\0')
            printf("*"); // debug
    }
    printf("\n");
}

char* obtener_texto(int bloque_inicial, int tamanio, int puntero_archivo) {
    
    int inicio_bloque = obtener_inicio_bloque(bloque_inicial);

    char* texto = malloc(tamanio + 1);

    memcpy(texto, &bufferBloques[inicio_bloque + puntero_archivo], tamanio);

    texto[tamanio] = '\0';

    return texto;
}

int buscar_espacio_bitmap(int tamanio){
    
    int contador_bloques;

    int tam_bitmap = bitarray_get_max_bit(bufferBitmap);

    int bloques_requeridos = ceil((double)tamanio / tamanio_bloques);

    int nuevo_bloque;

    int vuelta = 0;
    
    do{
        nuevo_bloque = buscar_siguiente_bloque_libre();
        
        if(nuevo_bloque == -1){
            return -1;
        }
        
        contador_bloques = 0;

        for(int i = nuevo_bloque;i < nuevo_bloque + bloques_requeridos && i < tam_bitmap; i++){
            
            if(bitarray_test_bit(bufferBitmap, i) == false){
                contador_bloques++;
            }
        }

        if(contador_bloques == bloques_requeridos){
            return nuevo_bloque;
        }

        vuelta++;

    }while(vuelta < 2);

    return -1;
}

int obtener_inicio_bloque(int numero_bloque)
{

    if (numero_bloque < 0 || numero_bloque >= cantidad_bloques)
    {
        // El número de bloque está fuera de rango
        return -1;
    }

    return numero_bloque * tamanio_bloques;
}

static bool comparador_elementos(void* nombre_a, void* nombre_b) {

    char *path_a = string_from_format("%s/%s", ruta_metadata, (char*)nombre_a);
    char *path_b = string_from_format("%s/%s", ruta_metadata, (char*)nombre_b);

    t_config* config_a = config_create(path_a);
    t_config* config_b = config_create(path_b);

    int bloque_a = config_get_int_value(config_a, "BLOQUE_INICIAL");
    int bloque_b = config_get_int_value(config_b, "BLOQUE_INICIAL");
    
    free(path_a);
    free(path_b);
    config_destroy(config_a);
    config_destroy(config_b);

    return bloque_a < bloque_b;
}

int compactar(uint16_t pid, String archivo_truncar, int tamanio) {

    log_info(logger, "PID: %i, Inicio Compactacion.", pid);
    
    DIR* dir;

    struct dirent *entry;

    int tamanio_truncar, bloque_inicial_truncar;
    
    if(obtener_metadata(archivo_truncar, &bloque_inicial_truncar, &tamanio_truncar) == false){
        return false;
    }
    
    char* buffer_truncar = obtener_texto(bloque_inicial_truncar, tamanio_truncar, 0);

    //CREO LA LISTA
    t_list* lista_archivos = list_create();

    //obtener todos los archivos de la ruta

    dir = opendir(ruta_metadata);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return EXIT_FAILURE;
    }

    //agrego todos los archivos que tengo a la lista
    while ((entry = readdir(dir)) != NULL) {

        // Ignorar "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        list_add(lista_archivos, entry->d_name);

    } 

    //ordeno la lista segun BLOQUE_INICIAL
    list_sort(lista_archivos, comparador_elementos);

    //obtengo el tamanio de la lista
    int tamanio_lista = list_size(lista_archivos);

    //obtengo el maximo de sus elementos
    int tamanio_bitarray = bitarray_get_max_bit(bufferBitmap);

    //lo recorro y voy limpiando
    for(int i = 0; i < tamanio_bitarray; i++){
        bitarray_clean_bit(bufferBitmap, i);
    }
    
    //creo nuevos bloques
    char* bloques_auxiliares = inicializar_bloques("bloques_auxiliares.dat");
    
    char* nombre_archivo;
    char* buffer;

    int tamanio_archivo, bloque_inicial, nuevo_bloque;

    int bloques_usados;

    for(int i = 0; i < tamanio_lista; i++){
        
        nuevo_bloque = buscar_primer_bloque_libre(); //devuelve la primera pos libre

        if(nuevo_bloque < 0)
            return ERROR;

        nombre_archivo = list_get(lista_archivos, i);

        // salteo el archivo a truncar
        if(strcmp(nombre_archivo, archivo_truncar) == 0){
            continue;
        }

        //obtengo el tam_archivo, bloq_inicial y copio su contenido a bloques_auxiliares
        if(obtener_metadata(nombre_archivo, &bloque_inicial, &tamanio_archivo) == false){
            return false;
        }

        buffer = obtener_texto(bloque_inicial, tamanio_archivo, 0);

        //escribo en bloques_auxiliares
        escribir_texto_en_bloques(bloques_auxiliares, nuevo_bloque, tamanio_archivo, 0, buffer); // tengo que escribirlo en bloques_auxiliares

        bloques_usados = ceil((double)tamanio_archivo / (double)tamanio_bloques);

        for(int i = nuevo_bloque; i < nuevo_bloque + bloques_usados; i++){
            bitarray_set_bit(bufferBitmap, i);
        }

        //modifico la metadata
        if(guardar_metadata(nombre_archivo, nuevo_bloque, tamanio_archivo) == false){
            return false;
        }
        
        free(buffer);
    }

    // int resultado = buscar_espacio_bitmap(tamanio);
    
    // if(resultado == -1){
    //     printf("No hay espacio suficiente luego de la compactacion\n");
    //     return resultado;
    // }

    nuevo_bloque = buscar_primer_bloque_libre();

    //escribo el contenido del archivo a truncar
    escribir_texto_en_bloques(bloques_auxiliares, nuevo_bloque, tamanio, 0, buffer_truncar); // con en tamanio solicitado

    bloques_usados = ceil((double) tamanio / (double) tamanio_bloques);

    for(int i = nuevo_bloque; i < nuevo_bloque + bloques_usados; i++){
            bitarray_set_bit(bufferBitmap, i);
    }

    //seteo sus nuevos valores
    if(guardar_metadata(archivo_truncar, nuevo_bloque, tamanio) == false){
        return false;
    }

    //tengo que reemplazar el contendido de bufferBloques por bloques_auxiliares
    memcpy(bufferBloques, bloques_auxiliares, tamanio_bloques * cantidad_bloques);

    //sincronizo los datos de bufferBloques
    msync(bufferBloques, tamanio_bloques * cantidad_bloques, MS_SYNC);


    if(remove("bloques_auxiliares.dat") != 0){
        printf("No se pudo eliminar el archivo auxiliar de bloques\n");
    }

    list_destroy(lista_archivos);

    free(buffer_truncar);
    
    ESPERAR_X_MILISEGUNDOS(interfaz_config->retraso_compactacion);

    log_info(logger, "PID: %i, Fin Compactacion.", pid);

    return true;
}

///
void mostrar_bitmap(t_bitarray *bitmap)
{

    int tamanioPosta = bitarray_get_max_bit(bufferBitmap);

    for (int i = 0; i < tamanioPosta; i++)
    {
        bool valor_bit = bitarray_test_bit(bufferBitmap, i);
        printf("Valor del bit[%i]: %d\n", i, valor_bit);
    }
}

int obtener_metadata(String nombre_archivo, int* bloque_inicial, int* tamanio_archivo){
    
    String path_archivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);

    if(access(path_archivo, F_OK) == -1 ){
        
        printf("No existe el archivo %s\n", nombre_archivo);
        
        free(path_archivo);
        
        return false;
    }

    t_config* metadata_archivo = config_create(path_archivo);


    if (metadata_archivo == NULL) {
        
        printf("No se pudo abrir la metadata del archivo: %s\n", nombre_archivo);
        
        free(path_archivo);
        
        return false;
    }
    
    *bloque_inicial = config_get_int_value(metadata_archivo, "BLOQUE_INICIAL");
    *tamanio_archivo = config_get_int_value(metadata_archivo, "TAMANIO_ARCHIVO");

    
    config_destroy(metadata_archivo);
    
    free(path_archivo);

    return true;

}

int guardar_metadata(String nombre_archivo, int bloque_inicial, int tamanio_archivo){

    String path_archivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);

    if(access(path_archivo, F_OK) == -1 ){
        
        printf("No existe el archivo %s\n", nombre_archivo);
        
        free(path_archivo);
        
        return false;
    }

    t_config* metadata_archivo = config_create(path_archivo);

    if (metadata_archivo == NULL) {
        printf("No se pudo abrir la metadata del archivo: %s\n", nombre_archivo);
        
        free(path_archivo);
        
        return false;
    }

    config_set_value(metadata_archivo, "BLOQUE_INICIAL", string_itoa(bloque_inicial));
    config_set_value(metadata_archivo, "TAMANIO_ARCHIVO", string_itoa(tamanio_archivo));

    config_save(metadata_archivo);
    free(path_archivo);
    config_destroy(metadata_archivo);

    return true;
}


int comprobar_espacio_libre(int bloques_requeridos){

    int tamanio_bitmap = bitarray_get_max_bit(bufferBitmap);
    int contador_bloques = 0;

    for(int i = 0; i < tamanio_bitmap; i++){
        if(bitarray_test_bit(bufferBitmap, i) == false){
            contador_bloques++;
        }
    
        if(contador_bloques == bloques_requeridos){
            return true;
        }
    }


    return false;
}