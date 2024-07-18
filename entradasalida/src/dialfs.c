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

    if(bufferBitmap == NULL || bufferBloques == NULL ){
        log_info(logger, "No se pudo montar en memoria el fileSystem");
        exit(EXIT_FAILURE);
    }

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

        paquete = recibir_paquete(fd_kernel);
        
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

            truncar_archivo(pid, nombre_archivo, tamanio);
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

    char* buffer_bloques = mmap(NULL, tamanioArchivo, PROT_WRITE | PROT_READ, MAP_SHARED, fd_bloques, 0);
    // mmap(NULL, tamanioArchivo, permisos ,MAP_SHARED,archivo , puntero(posicion))

    if (buffer_bloques == MAP_FAILED)
    {

        log_info(extra_logger, "Error al mapear el archivo bloques.dat");

        close(fd_bloques);
        free(bufferBloques);

        return NULL;
    }

    log_info(extra_logger, "Se monto en memoria el archivo bloques.dat");

    close(fd_bloques);

    return buffer_bloques;
}

t_bitarray* inicializar_bitmap(String ruta_bitmap)
{
    int tamanioBitmap = (cantidad_bloques + 7) / 8; // ceil(cantidad_bloques / 8);

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

    // for (int i = 0; i < tamanioBitmap; i++)
    // {
    // 	bitarray_clean_bit(bufferBitmap, i);
    // }

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
        exit(EXIT_FAILURE);
    }

    bitarray_set_bit(bufferBitmap, bloque_inicial);

    // abre el archivo en forma w para crearlo y lo cierra
    FILE *nuevofcb = fopen(path, "w");
    fclose(nuevofcb);

    // lo abre como config para cargarle los datos principales
    t_config *nuevoArchivo = iniciar_config(path);
    config_set_value(nuevoArchivo, "BLOQUE_INICIAL", string_itoa(bloque_inicial));
    config_set_value(nuevoArchivo, "TAMANIO_ARCHIVO", string_itoa(0));
    config_save(nuevoArchivo);
    config_destroy(nuevoArchivo);
    free(path);

    log_info(logger, "PID: %d, Crear Archivo: %s", pid, nombre_archivo);
    return;
}

void eliminar_archivo(uint16_t pid, String nombre_archivo)
{

    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);

    if (access(pathArchivo, F_OK) == -1)
    {
        printf("No se puedo eliminar, no existe el archivo\n");
        exit(EXIT_FAILURE);
    }

    t_config *valores = config_create(pathArchivo);

    int bloque_inicial = config_get_int_value(valores, "BLOQUE_INICIAL");
    int tamanio_archivo = config_get_int_value(valores, "TAMANIO_ARCHIVO");

    liberar_espacio_bitmap(bloque_inicial, tamanio_archivo);
    limpiar_bloques(bloque_inicial, tamanio_archivo);

    if (remove(pathArchivo) != 0)
    {
        printf("No se puedo eliminar el archivo %s\n", nombre_archivo);

        config_destroy(valores);
        free(pathArchivo);
        exit(EXIT_FAILURE);
    }

    config_destroy(valores);
    free(pathArchivo);

    log_info(logger, "PID: %d, Eliminar Archivo: %s", pid, nombre_archivo);
    return;
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

    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);

    if(access(pathArchivo, F_OK) == -1 ){
       printf("No existe la ruta especificada\n");
        exit(EXIT_FAILURE);
    }

    t_config *metadata_archivo = config_create(pathArchivo);

    if (metadata_archivo == NULL) {
        printf("No se puedo obtener la informacion del archivo: %s\n", nombre_archivo);
        free(pathArchivo);
        return;
    }

    int bloque_inicial = config_get_int_value(metadata_archivo, "BLOQUE_INICIAL");
    int tamanio_archivo = config_get_int_value(metadata_archivo, "TAMANIO_ARCHIVO");

    if (puntero_archivo >= tamanio_archivo) {
        printf("El puntero de archivo está fuera del tamaño del archivo: %s\n", nombre_archivo);
        config_destroy(metadata_archivo);
        free(pathArchivo);
        return;
    }

    // obtener el texto de los bloques
    char* texto_a_escribir = obtener_texto(bloque_inicial, tamanio, puntero_archivo);

    // envio el texto a memoria
    if(enviar_a_memoria(pid, direccion, texto_a_escribir) == false){
        printf("No se pudo escribir en memoria el texto");
        free(pathArchivo);
        exit(EXIT_FAILURE);
    }

    printf("Texto: %s\n", texto_a_escribir);
    printf("PID: %d, Leer Archivo: %s Tamaño a Leer: %d Puntero Archivo: %d\n", pid, nombre_archivo, tamanio, puntero_archivo);

    config_destroy(metadata_archivo);
    free(texto_a_escribir);
    free(pathArchivo);

    return;
}

void truncar_archivo(uint16_t pid, String nombre_archivo, int tamanio) {
    
    // verifico que sea una ruta valida
    char *pathArchivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);
    
    if(access(pathArchivo, F_OK) == -1 ){
        printf("No existe el archivo\n");
        free(pathArchivo);
        exit(EXIT_FAILURE);
    }
    
    // creo el configo y obtengo bloque inicial y tamanio
    t_config *metadata_archivo = config_create(pathArchivo);

    if (metadata_archivo == NULL) {
        printf("No se pudo abrir el archivo para truncar: %s\n", nombre_archivo);
        free(pathArchivo);
        exit(EXIT_FAILURE);
    }

    int bloque_inicial = config_get_int_value(metadata_archivo, "BLOQUE_INICIAL");
    int tamanio_actual = config_get_int_value(metadata_archivo, "TAMANIO_ARCHIVO");
    
    // calculo los bloques requeridos y los actuales
    int bloques_requeridos = ceil((double)tamanio / (double)tamanio_bloques);
    int bloques_actuales = ceil((double)tamanio_actual / (double)tamanio_bloques);

    // si el archivo lo tengo que hacer mas chico
    if(bloques_actuales > bloques_requeridos){

        int bloques_eliminar = bloques_actuales - bloques_requeridos;
        int offset = bloque_inicial + bloques_requeridos;

        liberar_espacio_bitmap(offset, bloques_eliminar); // verificar
        limpiar_bloques(offset, bloques_eliminar * tamanio_bloques); // testeo

        // marco como como ocupado en el bitmap
        for(int i = bloque_inicial; i < bloque_inicial + bloques_requeridos; i++){
            bitarray_set_bit(bufferBitmap, i);
        }

        //asigno el nuevo TAMANIO_ARCHIVO
        config_set_value(metadata_archivo, "TAMANIO_ARCHIVO", string_itoa(tamanio));

        config_save(metadata_archivo);
        config_destroy(metadata_archivo);
        free(pathArchivo);

        printf("PID: %d, Truncar Archivo: %s Tamaño: %d\n", pid, nombre_archivo, tamanio);
        return;
    }
    // si el archivo lo tengo que hacer mas grande
    else if(bloques_actuales < bloques_requeridos){

        bitarray_clean_bit(bufferBitmap, bloque_inicial);
        
        int nuevo_bloque = buscar_espacio_bitmap(tamanio);

        if(nuevo_bloque == -1){

            printf("No hay espacio suficiente para recolocar el archivo, realizando compactacion\n");

            int compactar_resultado = compactar(pid, nombre_archivo, tamanio);

            if(compactar_resultado == -1){
                printf("NO hay espacio");
                exit(EXIT_FAILURE);
            }

            return;
        }

        if(nuevo_bloque != bloque_inicial){
            
            // si tengo que recolocar el archivo tambien su contenido, y liberar el espacio anterior
            char* texto = obtener_texto(bloque_inicial, tamanio_actual, 0);

            liberar_espacio_bitmap(bloque_inicial, tamanio_actual);
            limpiar_bloques(bloque_inicial, tamanio_actual);

            escribir_texto_en_bloques(bufferBloques, nuevo_bloque, tamanio, 0, texto);

        }else{

            // lo unico que tendria que hacer es marcar los nuevo bloques ocupados
            for(int i = nuevo_bloque; i < nuevo_bloque + bloques_requeridos; i++){
                bitarray_set_bit(bufferBitmap, i);
            }
        }


        config_set_value(metadata_archivo, "BLOQUE_INICIAL", string_itoa(nuevo_bloque));
        config_set_value(metadata_archivo, "TAMANIO_ARCHIVO", string_itoa(tamanio));
        config_save(metadata_archivo);
        config_destroy(metadata_archivo);
        free(pathArchivo);

        printf("PID: %d, Truncar Archivo: %s Tamaño: %d\n", pid, nombre_archivo, tamanio);

        return;
    }
    else{
        printf("El archivo ya tiene ese tamaño.\n");
        return;
    }
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

    while(vuelta < 2){

        for (int i = ultima_posicion; i < tam; i++) {
            bool pos = bitarray_test_bit(bufferBitmap, i);
            if (pos == false) {
                ultima_posicion = i + 1;
                return i;
            }
        }

        ultima_posicion = 0;
        vuelta++;
    }

    return -1;
}

void liberar_espacio_bitmap(int bloque_inicial, int tamanio_eliminar)
{

    tamanio_eliminar = tamanio_eliminar * tamanio_bloques;

    int cant_bloques = ceil((double)tamanio_eliminar / (double)tamanio_bloques) + bloque_inicial;

    if (cant_bloques == bloque_inicial)
    {
        bitarray_clean_bit(bufferBitmap, bloque_inicial);
    }
    else
    {
        for (int i = bloque_inicial; i < cant_bloques; i++)
        {
            bitarray_clean_bit(bufferBitmap, i);
        }
    }
}

void limpiar_bloques(int primer_bloque, int tamanio_limpiar)
{
    int bloque_inicial = obtener_inicio_bloque(primer_bloque);
    for (int i = bloque_inicial; i < bloque_inicial + tamanio_limpiar; i++)
    {
        memset(&bufferBloques[i * tamanio_bloques], '\0', tamanio_bloques);
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

    int bloques = ceil((double)tamanio_archivo / (double)tamanio_bloques);
    int total = primer_bloque + bloques;

    // marco el bitmap
    for (int i = primer_bloque; i < total; i++)
    {
        bitarray_set_bit(bufferBitmap, i);
        // printf("Bloque[%d]: marcado como ocupado\n", i);
    }

    memcpy(&puntero_a_bloques[offset], texto, longitud_texto);

    msync(puntero_a_bloques, tamanio_bloques * cantidad_bloques, MS_SYNC);

    printf("Texto guardado en bloques correctamente.\n");
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

    int bloques_requeridos = ceil((double)tamanio / (double)tamanio_bloques);

    int nuevo_bloque;
    
    int vuelta = 0;
    
    do {
        nuevo_bloque = buscar_siguiente_bloque_libre();

        if (nuevo_bloque == -1) {
            return -1;
        }

        contador_bloques = 0;

        for (int i = nuevo_bloque; i < nuevo_bloque + bloques_requeridos && i < tam_bitmap; i++) {
            if (!bitarray_test_bit(bufferBitmap, i)) {
                contador_bloques++;
            }
        }

        if (contador_bloques == bloques_requeridos) {
            return nuevo_bloque;
        }

        vuelta++;
        
    } while (vuelta < 2);

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

    log_info(logger, "PID: %i, Inicio Compactación.", pid);

    DIR* dir;

    struct dirent *entry;

    //obtengo los datos del archivo a truncar
    char* path_archivo_truncar = string_from_format("%s/%s", ruta_metadata, archivo_truncar);
    
    t_config* config_archivo_truncar = config_create(path_archivo_truncar);
    
    int tamanio_truncar = config_get_int_value(config_archivo_truncar, "TAMANIO_ARCHIVO");
    int bloque_inicial_truncar = config_get_int_value(config_archivo_truncar, "BLOQUE_INICIAL");
    
    char* buffer_truncar = obtener_texto(bloque_inicial_truncar, tamanio_truncar, 0);

    //creo la lista
    t_list* lista_archivos = list_create();

    //obtener todos los archivos de la ruta
    char* nombre_archivo;

    dir = opendir(ruta_metadata);
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        return EXIT_FAILURE;
    }

    //agrego todos los archivos que tengo
    while ((entry = readdir(dir)) != NULL) {

        // Ignorar "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        list_add(lista_archivos, entry->d_name);

    } 

    //ordeno la lista segun el orden con BLOQUE_INICIAL
    list_sort(lista_archivos, comparador_elementos);

    // //busco la pos donde esta mi archivo
    int tamanio_lista = list_size(lista_archivos);

    //ordeno los archivos, libero el bitmap y voy recolocando el contenido el los bloques

    //limpio el bitmap
    int tamanio_bitarray = bitarray_get_max_bit(bufferBitmap);

    for(int i = 0; i < tamanio_bitarray; i++){
        bitarray_clean_bit(bufferBitmap, i);
    }

    // ordeno los archivos en el bitmap y en los bloques
    t_config* config_archivo;
    int nuevo_bloque;
    
    //creo nuevos bloques
    char* nuevos_bloques = inicializar_bloques("bloques_auxiliares.dat");

    char *path_archivo;
    char* buffer;
    int tamanio_archivo;
    int bloque_inicial;

    for(int i = 0; i < tamanio_lista; i++){
        
        nuevo_bloque = buscar_primer_bloque_libre(); //devuelve la primera pos libre

        if(nuevo_bloque < 0)
            return ERROR;

        nombre_archivo = list_get(lista_archivos, i);

        // salteo el archivo a truncar
        if(strcmp(nombre_archivo, archivo_truncar) == 0){
            continue;
        }

        //obtengo el tam_archivo, bloq_inicial y copio su contenido a nuevos_bloques
        path_archivo = string_from_format("%s/%s", ruta_metadata, nombre_archivo);

        config_archivo = config_create(path_archivo);

        tamanio_archivo = config_get_int_value(config_archivo, "TAMANIO_ARCHIVO");
        bloque_inicial = config_get_int_value(config_archivo, "BLOQUE_INICIAL");

        buffer = obtener_texto(bloque_inicial, tamanio_archivo, 0);

        //escribo en nuevos_bloques y se ocupa el bitmap
        escribir_texto_en_bloques(nuevos_bloques, nuevo_bloque, tamanio_archivo, 0, buffer); // tengo que escribirlo en nuevos_bloques

        //modifico la metadata
        config_set_value(config_archivo, "TAMANIO_ARCHIVO", string_itoa(tamanio_archivo));
        config_set_value(config_archivo, "BLOQUE_INICIAL", string_itoa(nuevo_bloque));

        //guardo
        config_save(config_archivo);

        //libero y paso al siguiente
        config_destroy(config_archivo);
        free(path_archivo);
        free(buffer);
    }
    
    int resultado = buscar_espacio_bitmap(tamanio);
    
    if(resultado == -1){
        printf("No hay espacio suficiente luego de la compactacion\n");
        return resultado;
    }

    nuevo_bloque = buscar_primer_bloque_libre();

    //escribo el contenido del archivo a truncar
    escribir_texto_en_bloques(nuevos_bloques, nuevo_bloque, tamanio, 0, buffer_truncar); // con en tamanio solicitado

    //seteo sus nuevos valores
    config_set_value(config_archivo_truncar, "TAMANIO_ARCHIVO", string_itoa(tamanio));
    config_set_value(config_archivo_truncar, "BLOQUE_INICIAL", string_itoa(nuevo_bloque));

    config_save(config_archivo_truncar);

    //mostrar_bitmap(bufferBitmap); // TESTEO

    //tengo que reemplazar el contendido de bufferBloques por nuevos_bloques
    memcpy(bufferBloques, nuevos_bloques, tamanio_bloques * cantidad_bloques);

    //sincronizo los datos de bufferBloques
    msync(bufferBloques, tamanio_bloques * cantidad_bloques, MS_SYNC);


    if(remove("bloques_auxiliares.dat") != 0){
        printf("No se pudo eliminar el archivo auxiliar de bloques\n");
    }

    //config_destroy(config_archivo);
    config_destroy(config_archivo_truncar);

    list_destroy(lista_archivos);

    free(path_archivo_truncar);
    free(buffer_truncar);

    // Luego de compactar el FS, se deberá esperar un tiempo determinado
    ESPERAR_X_MILISEGUNDOS(interfaz_config->retraso_compactacion);
    
    log_info(logger, "PID: %i,Fin Compactación.",pid);

    return resultado;
}
