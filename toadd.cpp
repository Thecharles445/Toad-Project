
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "conexion.h" // Usamos el nuevo nombre

using namespace std;

int main() {
    // Borramos tubos viejos por si acaso y los creamos de nuevo
    unlink(RUTA_TUBO_ENTRADA);
    unlink(RUTA_TUBO_SALIDA);
    mkfifo(RUTA_TUBO_ENTRADA, 0666);
    mkfifo(RUTA_TUBO_SALIDA, 0666);

    cout << "Gestor toadd (IID 1) iniciado y escuchando..." << endl;

    while (true) {
        // Abrimos el tubo de entrada para leer comandos
        int tubo = open(RUTA_TUBO_ENTRADA, O_RDONLY);
        
        if (tubo != -1) {
            PaqueteDatos datos;
            // Leemos lo que envio el CLI
            read(tubo, &datos, sizeof(PaqueteDatos));
            
            cout << "Comando recibido numero: " << datos.orden << endl;
            
            if (datos.orden == 1) {
                cout << "Ejecutando programa en: " << datos.ruta_programa << endl;
            }

            close(tubo);
        }
        
        // Espera un momento para no usar todo el procesador
        usleep(100000); 
    }
    return 0;
}