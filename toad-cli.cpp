#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "conexion.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Faltan argumentos" << endl;
        return 1;
    }

    PaqueteDatos peticion;
    memset(&peticion, 0, sizeof(PaqueteDatos));

    string comando = argv[1];

    // identificar que comando metio el usuario
    if (comando == "start") {
        if (argc < 3) { cout << "Falta binario" << endl; return 1; }
        peticion.orden = 1;
        strcpy(peticion.ruta_programa, argv[2]);
    } 
    else if (comando == "stop") {
        if (argc < 3) { cout << "Falta IID" << endl; return 1; }
        peticion.orden = 2;
        peticion.id_interno = stoi(argv[2]);
    }
    else if (comando == "ps") {
        peticion.orden = 3;
    }
    else if (comando == "status") {
        if (argc < 3) { cout << "Falta IID" << endl; return 1; }
        peticion.orden = 4;
        peticion.id_interno = stoi(argv[2]);
    }
    else if (comando == "kill") {
        if (argc < 3) { cout << "Falta IID" << endl; return 1; }
        peticion.orden = 5;
        peticion.id_interno = stoi(argv[2]);
    }
    else if (comando == "zombie") {
        peticion.orden = 6;
    }
    else {
        cout << "Comando invalido" << endl;
        return 1;
    }

    // escribir al gestor
    int tubo_in = open(TUBO_IN, O_WRONLY);
    if (tubo_in == -1) {
        cout << "Error: toadd no esta corriendo" << endl;
        return 1;
    }
    write(tubo_in, &peticion, sizeof(PaqueteDatos));
    close(tubo_in);

    // esperar y leer respuesta del gestor
    int tubo_out = open(TUBO_OUT, O_RDONLY);
    if (tubo_out != -1) {
        RespuestaDatos respuesta;
        read(tubo_out, &respuesta, sizeof(RespuestaDatos));
        cout << respuesta.mensaje << endl;
        close(tubo_out);
    }

    return 0;
}