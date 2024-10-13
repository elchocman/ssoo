#pragma once
#include "../osrms_File/Osrms_File.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef OSRMS_API_H
#define OSRMS_API_H

#include <stdio.h>
#include <string.h>

#define MAX_PROCESOS 32              // Máximo número de procesos
#define MAX_NOMBRE 11                // 11 Bytes para el nombre del proceso
#define MAX_ARCHIVOS 5               // Cada proceso puede manejar hasta 5 archivos
#define TAMAÑO_NOMBRE_ARCHIVO 14     // 14 Bytes para el nombre del archivo
#define MAX_TAMAÑO_ARCHIVO (64 * 1024 * 1024)  // Tamaño máximo del archivo: 64 MB
#define TAMAÑO_TABLA_SEGUNDO_ORDEN 1024  // Tamaño de la tabla de segundo orden (4KB = 1024 entradas de 4 bytes)
#define TAMAÑO_FRAME 32768           // Tamaño del frame en bytes (32KB)
#define TAMAÑO_TABLA_PRIMER_ORDEN 128  // Tamaño de la tabla de primer orden (128 bytes)

// Estructura para almacenar un archivo en la Tabla de Archivos
typedef struct {
    char validez;                           // 1 Byte: 0x01 si el archivo es válido, 0x00 si no
    char nombre[TAMAÑO_NOMBRE_ARCHIVO];     // 14 Bytes: Nombre del archivo
    int tamaño;                             // 4 Bytes: Tamaño del archivo en bytes (máx 64 MB)
    unsigned int direccion_virtual;         // 4 Bytes: Dirección virtual (5 bits no usados + 12 bits VPN + 15 bits offset)
} Archivo;

// Estructura para representar un archivo abierto
typedef struct {
    int process_id;                         // ID del proceso que posee el archivo
    char nombre[TAMAÑO_NOMBRE_ARCHIVO];     // Nombre del archivo
    int tamaño;                             // Tamaño del archivo
    unsigned int direccion_virtual;         // Dirección virtual del archivo
    char modo;                              // Modo: 'r' para lectura, 'w' para escritura
} osrmsFile;

// Estructura para el PCB (Process Control Block)
typedef struct {
    char estado;                // 1 Byte: Estado del proceso (0x01 activo, 0x00 inactivo)
    char id_proceso;            // 1 Byte: ID del proceso
    char nombre[MAX_NOMBRE];    // 11 Bytes: Nombre del proceso
    Archivo archivos[MAX_ARCHIVOS]; // 115 Bytes: Tabla de Archivos (5 archivos máximo)
    unsigned int tabla_paginas[32]; // 128 Bytes: Tabla de Páginas de Primer Orden (32 entradas de 4 bytes cada una)
} PCB;




// Declaraciones de funciones
void os_mount(char *memory_path);
void os_start_process(int id_proceso, char *nombre_proceso);
void os_ls_processes();
void os_finish_process(int id_proceso);

int os_write_file(osrmsFile* file_desc, char* src);
int os_read_file(osrmsFile* file_desc, char* dest);
int os_exists(int process_id, char* file_name);
void os_ls_files(int process_id);
void os_frame_bitmap();
void os_tp_bitmap();

int buscar_frame_libre();
void liberar_frames_asignados(unsigned int direccion_virtual, int tamaño);
void asignar_tabla_pagina(int process_id);
void liberar_tabla_pagina(int id_proceso);
void os_unmount();  // Declaración de la función para desmontar la memoria
void liberar_frames_del_proceso(int process_id);

// Función para abrir archivos (r: lectura, w: escritura)
osrmsFile* os_open(int process_id, char* file_name, char mode);

// Estructuras y funciones adicionales para manejar las Tablas de Páginas de Primer y Segundo Orden

// Estructura para la tabla de páginas de segundo orden (cada entrada apunta a un frame)
typedef struct {
    unsigned int frames[TAMAÑO_TABLA_SEGUNDO_ORDEN];  // Cada entrada tiene la dirección de un frame
} TablaPaginaSegundoOrden;

// Funciones para manejo de las tablas de segundo orden
TablaPaginaSegundoOrden* crear_tabla_segundo_orden();
void asignar_frame_a_tabla_segundo_orden(TablaPaginaSegundoOrden* tabla, unsigned int frame_num, int offset);
void liberar_tabla_segundo_orden(TablaPaginaSegundoOrden* tabla);

#endif // OSRMS_API_H
