#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "conexion.h" // Usamos el nuevo nombre

using namespace std;

int main(int argc, char* argv[]) {
    // Validamos que el usuario escriba al menos la orden
    if (argc < 2) {
        cout << "Uso: ./toad-cli <orden> [ruta]" << endl;
        return 1;
    }

    PaqueteDatos mi_envio;

    // Si el comando es "start"
    if (strcmp(argv[1], "start") == 0) {
        mi_envio.orden = 1;
        if (argc > 2) {
            strcpy(mi_envio.ruta_programa, argv[2]);
        }
    }

    // Abrimos el tubo de entrada del gestor y mandamos los datos
    int tubo = open(RUTA_TUBO_ENTRADA, O_WRONLY);
    if (tubo != -1) {
        write(tubo, &mi_envio, sizeof(PaqueteDatos));
        close(tubo);
        cout << "Orden enviada al gestor." << endl;
    } else {
        cout << "Error: No se pudo conectar con el gestor." << endl;
    }

    return 0;
}
