#include "../osrms_API/osrms_API.h"
#include <stdio.h>

#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <archivo_memoria>\n", argv[0]);
        return 1;
    }

    // Montar la memoria
    os_mount(argv[1]);

    // Crear y verificar dos procesos
    os_start_process(1, "Proceso1");
    os_start_process(2, "Proceso2");
    os_ls_processes();  // Listar procesos activos

    // Crear archivos en ambos procesos
    osrmsFile *archivo1 = os_open(1, "archivo1.txt", 'w');
    if (archivo1 != NULL) {
        int bytes_escritos1 = os_write_file(archivo1, "source_file1.txt");
        if (bytes_escritos1 > 0) {
            printf("Proceso 1: %d bytes escritos en archivo1.txt\n", bytes_escritos1);
        } else {
            printf("Error al escribir en archivo1.txt del proceso 1.\n");
        }
        os_close(archivo1);
    }

    osrmsFile *archivo2 = os_open(2, "archivo2.txt", 'w');
    if (archivo2 != NULL) {
        int bytes_escritos2 = os_write_file(archivo2, "source_file2.txt");
        if (bytes_escritos2 > 0) {
            printf("Proceso 2: %d bytes escritos en archivo2.txt\n", bytes_escritos2);
        } else {
            printf("Error al escribir en archivo2.txt del proceso 2.\n");
        }
        os_close(archivo2);
    }

    // Listar los archivos de cada proceso
    os_ls_files(1);
    os_ls_files(2);

    // Leer archivo del proceso 1
    osrmsFile *archivo_leido1 = os_open(1, "archivo1.txt", 'r');
    if (archivo_leido1 != NULL) {
        os_read_file(archivo_leido1, "dest_file1.txt");
        os_close(archivo_leido1);
    }

    // Liberar recursos y terminar procesos
    os_finish_process(1);
    os_finish_process(2);

    // Verificar el estado de los frames y tablas de páginas después de liberar
    os_frame_bitmap();
    os_tp_bitmap();

    // Desmontar la memoria
    os_unmount();

    return 0;
}
