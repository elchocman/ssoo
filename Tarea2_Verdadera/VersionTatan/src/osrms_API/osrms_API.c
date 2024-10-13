#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "osrms_API.h"


#include <stdio.h>
#include <stdlib.h>
#include "osrms_API.h"

#define MAX_PROCESOS 32 // Definimos un número máximo de procesos

// Simulación de la tabla de PCBs
PCB tabla_PCB[MAX_PROCESOS];

// Puntero global al archivo de memoria
FILE *memory_file = NULL;

const size_t TAMAÑO_TOTAL_MEMORIA = (size_t) 2 * 1024 * 1024 * 1024;  // 2 GB de memoria


void os_mount(char *memory_path) {
    // Abrir el archivo de memoria en modo lectura/escritura binario
    memory_file = fopen(memory_path, "rb+");
    
    if (memory_file == NULL) {
        fprintf(stderr, "Error: No se pudo abrir el archivo de memoria %s\n", memory_path);
        exit(EXIT_FAILURE);
    }

    printf("Memoria montada correctamente desde %s\n", memory_path);
}


void os_unmount() {
    if (memory_file != NULL) {
        fclose(memory_file);  // Cerrar el archivo de memoria
        printf("Archivo de memoria cerrado correctamente.\n");
        memory_file = NULL;  // Asegurarnos de que el puntero apunte a NULL
    }
}


void os_start_process(int id_proceso, char *nombre_proceso) {
    // Verificar si el proceso ya existe
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01 && tabla_PCB[i].id_proceso == id_proceso) {
            printf("Error: El proceso con ID %d ya existe.\n", id_proceso);
            return;
        }
    }

    // Buscar un espacio disponible en la tabla de PCBs
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x00) { // Si el proceso no está activo
            // Inicializamos el nuevo proceso
            tabla_PCB[i].estado = 0x01;  // Marcar el proceso como activo
            tabla_PCB[i].id_proceso = id_proceso;  // Asignar el ID del proceso
            strncpy(tabla_PCB[i].nombre, nombre_proceso, MAX_NOMBRE);  // Asignar el nombre del proceso

            // Inicializar la Tabla de Páginas (128 Bytes, vacía por ahora)
            memset(tabla_PCB[i].tabla_paginas, 0, sizeof(tabla_PCB[i].tabla_paginas));  // Inicializar la tabla de páginas

            // Asignar una Tabla de Páginas de Primer Orden
            asignar_tabla_pagina(id_proceso);

            // Escribir el PCB en los primeros 8 KB de la memoria (cuidado con el tamaño total)
            int pcb_offset = i * sizeof(PCB);  // Cada proceso tiene un espacio fijo en la memoria
            if (pcb_offset < 8 * 1024) {  // Asegurar que no sobrepasemos los 8 KB
                fseek(memory_file, pcb_offset, SEEK_SET);
                fwrite(&tabla_PCB[i], sizeof(PCB), 1, memory_file);  // Escribir el PCB en el archivo de memoria
                printf("Proceso %s (ID %d) iniciado correctamente y PCB escrito en memoria.\n", nombre_proceso, id_proceso);
            } else {
                printf("Error: No hay espacio para escribir el PCB en la memoria (8 KB excedidos).\n");
            }
            return;
        }
    }

    // Si no hay espacio en la tabla de PCBs
    printf("Error: No hay espacio disponible para iniciar el proceso.\n");
}



void os_ls_processes() {
    int procesos_activos = 0;

    printf("Lista de procesos activos:\n");

    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01) {  // Si el proceso está activo
            printf("ID: %d, Nombre: %s\n", tabla_PCB[i].id_proceso, tabla_PCB[i].nombre);
            procesos_activos++;
        }
    }

    if (procesos_activos == 0) {
        printf("No hay procesos activos en este momento.\n");
    }
}


void os_finish_process(int id_proceso) {
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01 && tabla_PCB[i].id_proceso == id_proceso) {
            // Liberar frames del proceso
            liberar_frames_del_proceso(id_proceso);
            
            // Liberar la tabla de páginas asociada
            liberar_tabla_pagina(id_proceso);

            // Marcar el proceso como inactivo
            tabla_PCB[i].estado = 0x00;

            // Escribir el PCB actualizado en la memoria, para reflejar que se eliminó
            int pcb_offset = i * sizeof(PCB);
            if (pcb_offset < 8 * 1024) {
                fseek(memory_file, pcb_offset, SEEK_SET);
                size_t written = fwrite(&tabla_PCB[i], sizeof(PCB), 1, memory_file);
                if (written != 1) {
                    printf("Error: No se pudo actualizar el PCB en la memoria.\n");
                } else {
                    printf("Proceso con ID %d ha sido terminado y PCB actualizado en la memoria.\n", id_proceso);
                }
            } else {
                printf("Error: Offset fuera de los primeros 8 KB de la memoria (no se puede eliminar el PCB).\n");
            }
            return;
        }
    }
    printf("Error: No se encontró el proceso con ID %d.\n", id_proceso);
}






osrmsFile* os_open(int process_id, char* file_name, char mode) {
    if (mode != 'r' && mode != 'w') {
        // Si el modo no es 'r' ni 'w', retornar NULL
        printf("Error: Modo inválido. Use 'r' para lectura o 'w' para escritura.\n");
        return NULL;
    }

    // Buscar el proceso por ID
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01 && tabla_PCB[i].id_proceso == process_id) {
            // Buscar el archivo por nombre
            for (int j = 0; j < MAX_ARCHIVOS; j++) {
                if (tabla_PCB[i].archivos[j].validez == 0x01 && 
                    strncmp(tabla_PCB[i].archivos[j].nombre, file_name, TAMAÑO_NOMBRE_ARCHIVO) == 0) {

                    if (mode == 'r') {
                        // Archivo encontrado, abrir para lectura
                        osrmsFile *archivo_abierto = (osrmsFile*) malloc(sizeof(osrmsFile));
                        archivo_abierto->process_id = process_id;
                        strncpy(archivo_abierto->nombre, file_name, TAMAÑO_NOMBRE_ARCHIVO);
                        archivo_abierto->tamaño = tabla_PCB[i].archivos[j].tamaño;
                        archivo_abierto->direccion_virtual = tabla_PCB[i].archivos[j].direccion_virtual;
                        archivo_abierto->modo = 'r';
                        return archivo_abierto;
                    } else if (mode == 'w') {
                        // El archivo ya existe, no se puede abrir para escritura
                        printf("Error: El archivo %s ya existe.\n", file_name);
                        return NULL;
                    }
                }
            }

            if (mode == 'w') {
                // Si el archivo no existe y estamos en modo 'w', creamos un archivo nuevo
                for (int j = 0; j < MAX_ARCHIVOS; j++) {
                    if (tabla_PCB[i].archivos[j].validez == 0x00) {  // Buscar una entrada libre
                        tabla_PCB[i].archivos[j].validez = 0x01;  // Marcar el archivo como válido
                        strncpy(tabla_PCB[i].archivos[j].nombre, file_name, TAMAÑO_NOMBRE_ARCHIVO);
                        tabla_PCB[i].archivos[j].tamaño = 0;  // Inicialmente el tamaño es 0
                        tabla_PCB[i].archivos[j].direccion_virtual = 0;  // Dirección virtual por asignar

                        // **Nueva parte:**
                        // Guardamos el PCB con el nuevo archivo en la memoria
                        int offset = i * sizeof(PCB);
                        fseek(memory_file, offset, SEEK_SET);
                        fwrite(&tabla_PCB[i], sizeof(PCB), 1, memory_file);  // Actualizamos el PCB en la memoria
                        
                        // Crear y retornar un osrmsFile* que representa el archivo
                        osrmsFile *archivo_creado = (osrmsFile*) malloc(sizeof(osrmsFile));
                        archivo_creado->process_id = process_id;
                        strncpy(archivo_creado->nombre, file_name, TAMAÑO_NOMBRE_ARCHIVO);
                        archivo_creado->tamaño = 0;
                        archivo_creado->direccion_virtual = tabla_PCB[i].archivos[j].direccion_virtual;
                        archivo_creado->modo = 'w';
                        return archivo_creado;
                    }
                }

                printf("Error: No hay espacio disponible en la tabla de archivos del proceso %d.\n", process_id);
                return NULL;
            }
        }
    }

    printf("Error: No se encontró el proceso con ID %d.\n", process_id);
    return NULL;
}



int os_exists(int process_id, char* file_name) {
    // Buscar el proceso por ID
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01 && tabla_PCB[i].id_proceso == process_id) {
            // Buscar el archivo por nombre en la tabla de archivos del proceso
            for (int j = 0; j < MAX_ARCHIVOS; j++) {
                if (tabla_PCB[i].archivos[j].validez == 0x01 &&
                    strncmp(tabla_PCB[i].archivos[j].nombre, file_name, TAMAÑO_NOMBRE_ARCHIVO) == 0) {
                    // Archivo encontrado
                    return 1;  // Existe
                }
            }
            // Archivo no encontrado
            printf("Archivo %s no encontrado en el proceso %d.\n", file_name, process_id);
            return 0;  // No existe
        }
    }
    // Proceso no encontrado
    printf("Error: No se encontró el proceso con ID %d.\n", process_id);
    return -1;  // Proceso no encontrado
}




void os_ls_files(int process_id) {
    // Buscar el proceso por ID
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01 && tabla_PCB[i].id_proceso == process_id) {
            printf("Archivos en el proceso %d:\n", process_id);
            int archivos_encontrados = 0;
            // Listar todos los archivos del proceso
            for (int j = 0; j < MAX_ARCHIVOS; j++) {
                if (tabla_PCB[i].archivos[j].validez == 0x01) {
                    printf("Archivo: %s, Tamaño: %d bytes\n", tabla_PCB[i].archivos[j].nombre, tabla_PCB[i].archivos[j].tamaño);
                    archivos_encontrados++;
                }
            }

            // Si no hay archivos válidos
            if (archivos_encontrados == 0) {
                printf("No hay archivos válidos en el proceso %d.\n", process_id);
            }
            return;  // Asegurarse de salir después de listar archivos
        }
    }
    printf("Error: No se encontró el proceso con ID %d.\n", process_id);
}





void os_frame_bitmap() {
    unsigned char frame_bitmap[8192];  // 8 KB para el bitmap de frames (65536 bits = 8192 bytes)

    // Moverse al lugar correcto en el archivo de memoria para leer el Frame Bitmap
    fseek(memory_file, (8 * 1024 + 128 + 128 * 1024 + 128), SEEK_SET);  // Calculamos el offset de acuerdo con la estructura de memoria
    size_t read_bytes = fread(frame_bitmap, sizeof(unsigned char), 8192, memory_file);  // Leemos los 8 KB del Frame Bitmap desde el archivo

    // Validar que se leyeron los 8192 bytes
    if (read_bytes != 8192) {
        fprintf(stderr, "Error: No se pudo leer el Frame Bitmap completo.\n");
        return;
    }

    int frames_ocupados = 0;
    int frames_libres = 0;

    // Recorrer todos los frames (65536 frames = 8192 bytes * 8 bits)
    for (int i = 0; i < 65536; i++) {
        int byte_index = i / 8;   // Obtener el índice del byte en el bitmap
        int bit_index = i % 8;    // Obtener el índice del bit dentro del byte

        // Verificar si el frame está ocupado o libre
        if (frame_bitmap[byte_index] & (1 << bit_index)) {
            frames_ocupados++;    // El frame está ocupado
        } else {
            frames_libres++;      // El frame está libre
        }
    }

    // Imprimir el estado del Frame Bitmap
    printf("Frames ocupados: %d\n", frames_ocupados);
    printf("Frames libres: %d\n", frames_libres);
}



// Función para escribir el archivo en los frames asignados
int os_write_file(osrmsFile *file_desc, char *src) {
    if (file_desc->modo != 'w') {
        printf("Error: El archivo %s no está abierto en modo escritura.\n", file_desc->nombre);
        return -1;
    }

    // Abrir el archivo real desde la ruta src
    FILE *src_file = fopen(src, "rb");
    if (src_file == NULL) {
        printf("Error: No se pudo abrir el archivo %s para lectura.\n", src);
        return -1;
    }

    // Obtener el tamaño del archivo
    fseek(src_file, 0, SEEK_END);
    int size = ftell(src_file);
    fseek(src_file, 0, SEEK_SET);

    if (size > MAX_TAMAÑO_ARCHIVO) {
        printf("Error: El tamaño del archivo excede el máximo permitido (64MB).\n");
        fclose(src_file);
        return -1;
    }

    // Leer el archivo completo en un buffer
    char *buffer = (char *)malloc(size);
    if (fread(buffer, 1, size, src_file) != size) {
        printf("Error: No se pudo leer el archivo completo.\n");
        fclose(src_file);
        free(buffer);
        return -1;
    }
    fclose(src_file);

    // Asegurarnos de que el proceso tenga una Tabla de Páginas asignada
    asignar_tabla_pagina(file_desc->process_id);

    // Calcular cuántos frames necesitamos
    int frames_needed = (size + 32767) / 32768;  // Tamaño de un frame es 32 KB, redondeamos hacia arriba
    int total_bytes_written = 0;  // Variable para almacenar los bytes realmente escritos

    // Asignar los frames necesarios
    for (int i = 0; i < frames_needed; i++) {
        int frame_libre = buscar_frame_libre();
        if (frame_libre == -1) {
            printf("Error: No hay frames libres disponibles.\n");
            free(buffer);
            return -1;
        }

        // Aquí escribimos en la memoria montada (el archivo binario)
        fseek(memory_file, frame_libre * 32768, SEEK_SET);  // Mover el puntero del archivo a la posición del frame libre

        int bytes_a_escribir = (size > 32768) ? 32768 : size;  // Determinar cuántos bytes escribir en este frame
        fwrite(buffer + (i * 32768), 1, bytes_a_escribir, memory_file);  // Escribir los datos al frame
        total_bytes_written += bytes_a_escribir;  // Acumular la cantidad de bytes escritos
        size -= bytes_a_escribir;

        printf("Escribiendo %d bytes en el frame %d\n", bytes_a_escribir, frame_libre);
    }

    free(buffer);  // Liberar el buffer

    // Actualizar el tamaño del archivo en el PCB
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01 && tabla_PCB[i].id_proceso == file_desc->process_id) {
            for (int j = 0; j < MAX_ARCHIVOS; j++) {
                if (tabla_PCB[i].archivos[j].validez == 0x01 &&
                    strncmp(tabla_PCB[i].archivos[j].nombre, file_desc->nombre, TAMAÑO_NOMBRE_ARCHIVO) == 0) {
                    // Actualizar el tamaño del archivo con el total de bytes escritos
                    tabla_PCB[i].archivos[j].tamaño += total_bytes_written;
                    break;
                }
            }
        }
    }

    printf("Archivo %s escrito correctamente con %d bytes.\n", file_desc->nombre, total_bytes_written);
    return total_bytes_written;  // Retornar la cantidad de bytes escritos
}




int buscar_frame_libre() {
    unsigned char frame_bitmap[8192];

    fseek(memory_file, (8 * 1024 + 128 + 128 * 1024 + 128), SEEK_SET);
    fread(frame_bitmap, sizeof(unsigned char), 8192, memory_file);

    for (int i = 0; i < 65536; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (!(frame_bitmap[byte_index] & (1 << bit_index))) {
            frame_bitmap[byte_index] |= (1 << bit_index);

            fseek(memory_file, (8 * 1024 + 128 + 128 * 1024 + 128), SEEK_SET);
            fwrite(frame_bitmap, sizeof(unsigned char), 8192, memory_file);

            printf("Frame %d asignado.\n", i);

            return i;
        }
    }

    return -1;
}



void liberar_frames_del_proceso(int process_id) {
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01 && tabla_PCB[i].id_proceso == process_id) {
            // Liberar los frames asociados a los archivos del proceso
            for (int j = 0; j < MAX_ARCHIVOS; j++) {
                if (tabla_PCB[i].archivos[j].validez == 0x01) {
                    liberar_frames_asignados(tabla_PCB[i].archivos[j].direccion_virtual, tabla_PCB[i].archivos[j].tamaño);
                }
            }
        }
    }
}


// Función auxiliar que liberará los frames según la dirección virtual y el tamaño del archivo
void liberar_frames_asignados(unsigned int direccion_virtual, int tamaño) {
    int frames_a_liberar = (tamaño + 32767) / 32768;
    unsigned char frame_bitmap[8192];

    fseek(memory_file, (8 * 1024 + 128 + 128 * 1024 + 128), SEEK_SET);
    fread(frame_bitmap, sizeof(unsigned char), 8192, memory_file);

    for (int i = 0; i < frames_a_liberar; i++) {
        int frame_libre = (direccion_virtual >> 15) + i;

        if (frame_libre * 32768 >= TAMAÑO_TOTAL_MEMORIA) {
            printf("Error: Intento de liberar frame fuera de los límites.\n");
            return;
        }

        int byte_index = frame_libre / 8;
        int bit_index = frame_libre % 8;

        frame_bitmap[byte_index] &= ~(1 << bit_index);
    }

    fseek(memory_file, (8 * 1024 + 128 + 128 * 1024 + 128), SEEK_SET);
    fwrite(frame_bitmap, sizeof(unsigned char), 8192, memory_file);
}



int os_read_file(osrmsFile* file_desc, char* dest) {
    if (file_desc->modo != 'r') {
        printf("Error: El archivo %s no está abierto en modo lectura.\n", file_desc->nombre);
        return -1;
    }

    // Abrir el archivo destino en el sistema de archivos local
    FILE *dest_file = fopen(dest, "wb");
    if (dest_file == NULL) {
        printf("Error: No se pudo abrir el archivo destino %s para escritura.\n", dest);
        return -1;
    }

    int size = file_desc->tamaño;
    unsigned char *buffer = (unsigned char*) malloc(32768);  // Buffer de 32 KB (tamaño de un frame)

    // Leer el archivo desde la memoria montada
    for (int i = 0; i < (size + 32767) / 32768; i++) {
        int frame_actual = (file_desc->direccion_virtual >> 15) + i;  // Obtener el frame actual
        fseek(memory_file, (frame_actual * 32768), SEEK_SET);
        int bytes_a_leer = (size > 32768) ? 32768 : size;
        fread(buffer, 1, bytes_a_leer, memory_file);

        fwrite(buffer, 1, bytes_a_leer, dest_file);  // Escribir en el archivo destino
        size -= bytes_a_leer;
    }

    fclose(dest_file);
    free(buffer);

    printf("Archivo %s leído correctamente en %s.\n", file_desc->nombre, dest);
    return file_desc->tamaño;
}



// Función para imprimir el estado del Bitmap de Tablas de Páginas
void os_tp_bitmap() {
    unsigned char tp_bitmap[128];  // 1024 bits = 128 bytes
    
    // Moverse al lugar correcto en el archivo de memoria para leer el Bitmap de Tablas de Páginas
    fseek(memory_file, (8 * 1024), SEEK_SET);  // Offset donde empieza el Bitmap de Tablas de Páginas
    size_t read_bytes = fread(tp_bitmap, sizeof(unsigned char), 128, memory_file);

    // Validar que se leyeron los 128 bytes esperados
    if (read_bytes != 128) {
        fprintf(stderr, "Error: No se pudo leer el Bitmap de Tablas de Páginas completo.\n");
        return;
    }

    int tablas_ocupadas = 0;
    int tablas_libres = 0;

    // Recorrer el bitmap para contar cuántas tablas están ocupadas y libres
    for (int i = 0; i < 1024; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (tp_bitmap[byte_index] & (1 << bit_index)) {
            tablas_ocupadas++;
        } else {
            tablas_libres++;
        }
    }

    // Imprimir el estado del Bitmap de Tablas de Páginas
    printf("Tablas de Páginas ocupadas: %d\n", tablas_ocupadas);
    printf("Tablas de Páginas libres: %d\n", tablas_libres);
}



// Declaración de un array global para el Bitmap de Tablas de Páginas
unsigned char bitmap_tablas_paginas[128];  // 128 Bytes = 1024 bits (una tabla por cada bit)


// Manejador de memoria global (simulación)
unsigned char *memoria;


// Función para asignar una Tabla de Páginas si el proceso no tiene una asignada
void asignar_tabla_pagina(int process_id) {

    // Supongamos que asignas una tabla de primer orden aquí
    printf("Asignando Tabla de Páginas de Primer Orden al proceso %d\n", process_id);
    // Verifica si efectivamente asignaste la tabla
    printf("Tabla de Páginas de Primer Orden asignada: %p\n", &(tabla_PCB[process_id].tabla_paginas));

    unsigned char tp_bitmap[128];  // 1024 bits = 128 bytes para el bitmap
    
    // Leer el Bitmap de Tablas de Páginas desde la memoria
    fseek(memory_file, (8 * 1024), SEEK_SET);  // Offset donde empieza el Bitmap de Tablas de Páginas
    fread(tp_bitmap, sizeof(unsigned char), 128, memory_file);

    // Buscar una tabla de páginas libre
    for (int i = 0; i < 1024; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (!(tp_bitmap[byte_index] & (1 << bit_index))) {  // Si el bit es 0, significa que está libre
            tp_bitmap[byte_index] |= (1 << bit_index);  // Marcar la tabla como ocupada

            // Escribir el cambio de vuelta en la memoria
            fseek(memory_file, (8 * 1024), SEEK_SET);
            fwrite(tp_bitmap, sizeof(unsigned char), 128, memory_file);

            // Asignar la tabla en la región correspondiente (128 KB después del Bitmap)
            unsigned char nueva_tabla[128];  // La nueva tabla de páginas vacía
            memset(nueva_tabla, 0, sizeof(nueva_tabla));
            fseek(memory_file, (8 * 1024 + 128 + i * 128), SEEK_SET);  // Espacio para la tabla de segundo orden
            fwrite(nueva_tabla, sizeof(nueva_tabla), 1, memory_file);  // Escribir la nueva tabla de páginas

            printf("Tabla de páginas asignada para el proceso %d.\n", process_id);
            return;
        }
    }

    printf("Error: No hay tablas de páginas libres disponibles.\n");
}



// Función para buscar una Tabla de Páginas libre en el Bitmap
int buscar_tabla_pagina_libre() {
    // Leer el Bitmap de Tablas de Páginas desde la memoria
    fseek(memory_file, (8 * 1024 + 128), SEEK_SET);  // Offset para el Bitmap
    fread(bitmap_tablas_paginas, sizeof(unsigned char), 128, memory_file);

    for (int i = 0; i < 1024; i++) {
        int byte_index = i / 8;
        int bit_index = i % 8;

        if (!(bitmap_tablas_paginas[byte_index] & (1 << bit_index))) {
            // Retornar el índice de la tabla libre
            return i;
        }
    }
    return -1;  // No hay tablas disponibles
}



// Función para marcar una tabla como ocupada en el Bitmap
void marcar_tabla_pagina_ocupada(int tabla) {
    int byte_index = tabla / 8;
    int bit_index = tabla % 8;

    // Marcar la tabla como ocupada en el Bitmap
    bitmap_tablas_paginas[byte_index] |= (1 << bit_index);

    // Guardar el Bitmap actualizado en el archivo de memoria
    fseek(memory_file, (8 * 1024 + 128), SEEK_SET);
    fwrite(bitmap_tablas_paginas, sizeof(unsigned char), 128, memory_file);
}



void liberar_tabla_pagina(int id_proceso) {
    unsigned char tp_bitmap[128];  // 1024 bits = 128 bytes

    // Leer el Bitmap de Tablas de Páginas desde la memoria
    fseek(memory_file, (8 * 1024), SEEK_SET);  // Offset donde empieza el Bitmap de Tablas de Páginas
    fread(tp_bitmap, sizeof(unsigned char), 128, memory_file);

    // Buscar el proceso en la tabla de PCBs y obtener la tabla de páginas
    for (int i = 0; i < MAX_PROCESOS; i++) {
        if (tabla_PCB[i].estado == 0x01 && tabla_PCB[i].id_proceso == id_proceso) {
            int tabla_pagina = (int)(tabla_PCB[i].tabla_paginas[0]);  // Obtener la tabla de páginas
            int byte_index = tabla_pagina / 8;
            int bit_index = tabla_pagina % 8;

            // Marcar la tabla como libre
            tp_bitmap[byte_index] &= ~(1 << bit_index);

            // Escribir el cambio de vuelta en la memoria
            fseek(memory_file, (8 * 1024), SEEK_SET);
            fwrite(tp_bitmap, sizeof(unsigned char), 128, memory_file);

            printf("Tabla de páginas del proceso %d liberada.\n", id_proceso);
            return;
        }
    }
}



// Función para cerrar un archivo
void os_close(osrmsFile* file_desc) {
    if (file_desc == NULL) {
        printf("Descriptor de archivo nulo.\n");
        return;
    }

    // Aquí no es necesario limpiar los campos manualmente, simplemente liberamos el descriptor
    free(file_desc);

    printf("Archivo cerrado correctamente.\n");
}


