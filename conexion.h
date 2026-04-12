#include <string>

using namespace std;

// Archivos temporales que serviran como tubos de comunicacion
const char* RUTA_TUBO_ENTRADA = "/tmp/tubo_entrada_tarea";
const char* RUTA_TUBO_SALIDA = "/tmp/tubo_salida_tarea";

// Estructura para enviar datos entre el CLI y el Gestor
struct PaqueteDatos {
    int orden;           // 1: start, 2: stop, etc.
    char ruta_programa[256]; 
    int id_interno;      // El IID asignado
};
