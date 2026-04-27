#ifndef CONEXION_H
#define CONEXION_H

// rutas para los pipes
const char* TUBO_IN = "/tmp/tubo_in_tarea1";
const char* TUBO_OUT = "/tmp/tubo_out_tarea1";

// estados que pide la tarea
enum EstadoProceso { RUNNING, STOPPED, ZOMBIE, FAILED };

// lo que envia el cli
struct PaqueteDatos {
    int orden; // 1:start, 2:stop, 3:ps, 4:status, 5:kill, 6:zombie
    char ruta_programa[256];
    int id_interno;
};

// lo que responde el gestor
struct RespuestaDatos {
    int iid_asignado;
    char mensaje[2048]; // buffer grande para salidas como ps o status
};

#endif