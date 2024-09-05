# Proyecto de Scheduler con Colas de Prioridad

## Compilación
Para compilar el proyecto, usa el comando `make` en el directorio raíz del proyecto.


## Ejecución
Para ejecutar el programa, proporciona un archivo de entrada con los datos de los procesos:
./lrscheduler input.txt

## Estructura del Proyecto
- **process**: Define la estructura y funciones para manejar los procesos.
- **queue**: Implementa una cola con prioridad para manejar los procesos.
- **file_manager**: Maneja la lectura y escritura de archivos de entrada.
- **main.c**: El archivo principal que conecta todos los módulos y ejecuta el scheduler.

## Formato del Archivo de Entrada
El archivo debe tener el siguiente formato:
1. La primera línea indica la cantidad de procesos.
2. Cada línea posterior define un proceso con los siguientes atributos:
    nombre pid burst_time bursts io_wait deadline


## Ejemplo de Archivo de Entrada:
    2 Proceso1 1 5 3 2 10 Proceso2 2 4 2 1 8