# Gestor de Procesos (toadd & toad-cli)

Este trabajo consiste en un sistema de administracion y monitoreo de procesos compuesto por dos componentes principales que se comunican mediante Pipes Nombrados (FIFOs):

1. **toadd**: Un demonio (daemon) que corre en segundo plano, independiente de la terminal. Se encarga de clonar, monitorear y controlar el ciclo de vida de los procesos hijos.
2. **toad-cli**: Una herramienta de linea de comandos para interactuar con el gestor y enviar órdenes.

Desarrollado para la asignatura de Sistemas Operativos bajo las restricciones de no utilizar threads ni mecanismos de sincronizacion avanzados, basandose puramente en el manejo de procesos y senales del sistema.

## Caracteristicas y Comandos

El cliente `toad-cli` soporta los siguientes comandos:

* **`start <bin_path>`**: Solicita al gestor ejecutar el binario indicado. Registra el proceso asignandole un Internal ID (IID) secuencial unico a partir del 2 (el IID 1 queda reservado para el gestor).
* **`stop <iid>`**: Envia una senal `SIGTERM` al proceso especificado por su IID para detenerlo de forma controlada.
* **`ps`**: Muestra una tabla con todos los procesos bajo administracion, detallando su IID, PID del sistema, estado actual, tiempo de ejecucion (uptime) y la ruta del binario.
* **`status <iid>`**: Muestra informacion detallada de un unico proceso, incluyendo la ruta, su estado exacto y la cantidad de reinicios automaticos que ha experimentado.
* **`kill <iid>`**: Termina de forma inmediata (`SIGKILL`) el proceso indicado y a toda su descendencia (arbol de procesos), previniendo la aparicion de procesos huerfanos.
* **`zombie`**: Lista exclusivamente aquellos procesos que han finalizado pero cuyo estado de salida aun no ha sido recolectado por el gestor.

### Bonus: Reinicio Automatico
Si un proceso en estado `RUNNING` termina de forma inesperada (sin una orden explicita de `stop` o `kill`), el gestor `toadd` lo relanzara de forma automatica manteniendo su IID original. Posee un limite configurable de 5 intentos consecutivos; si el proceso excede este limite, se marcara definitivamente con el estado `FAILED`.

## Requisitos de Compilacion

El codigo cumple estrictamente con el estandar solicitado y debe ser compilado utilizando:
* **Compilador**: `g++`
* **Flags**: `-Wall -Wextra -std=c++17`

## Instalacion y Uso

1. **Compilar los componentes:**
   ```bash
   g++ -Wall -Wextra -std=c++17 toadd.cpp -o toadd
   g++ -Wall -Wextra -std=c++17 toad-cli.cpp -o toad-cli
