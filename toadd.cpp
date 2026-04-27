#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <cstring>
#include <signal.h>
#include <time.h>
#include "conexion.h"

using namespace std;

const int MAX_REINICIOS = 5; // limite para el bonus

struct Proceso {
    int iid;
    pid_t pid;
    EstadoProceso estado;
    time_t tiempo_inicio;
    char ruta[256];
    int reinicios;
};

vector<Proceso> lista_procesos;
int siguiente_iid = 2; // IID 1 es toadd

string estadoToString(EstadoProceso e) {
    if (e == RUNNING) return "RUNNING";
    if (e == STOPPED) return "STOPPED";
    if (e == ZOMBIE) return "ZOMBIE";
    return "FAILED";
}

int main() {
    // daemonizar el programa
    pid_t pid_daemon = fork();
    if (pid_daemon < 0) return 1;
    if (pid_daemon > 0) return 0; // muere el padre
    setsid(); // crea nueva sesion, ya no depende de la terminal

    // limpiar y crear pipes
    unlink(TUBO_IN);
    unlink(TUBO_OUT);
    mkfifo(TUBO_IN, 0666);
    mkfifo(TUBO_OUT, 0666);

    // abrir tubo de lectura como no bloqueante
    int tubo_in = open(TUBO_IN, O_RDONLY | O_NONBLOCK);
    // abrir para escritura para evitar fin de archivo (EOF) infinito
    int dummy_fd = open(TUBO_IN, O_WRONLY); 

    while (true) {
        // 1. revisar estado y aplicar bonus de reinicio
        for (auto &p : lista_procesos) {
            int status;
            // WNOHANG para no trabar el programa
            pid_t res = waitpid(p.pid, &status, WNOHANG);
            
            if (res > 0) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    if (p.estado == RUNNING) {
                        // murio inesperadamente
                        if (p.reinicios < MAX_REINICIOS) {
                            // bonus: relanzar
                            pid_t nuevo_pid = fork();
                            if (nuevo_pid == 0) {
                                setpgid(0, 0);
                                execl(p.ruta, p.ruta, NULL);
                                exit(1);
                            } else if (nuevo_pid > 0) {
                                p.pid = nuevo_pid;
                                p.reinicios++;
                            }
                        } else {
                            p.estado = FAILED;
                        }
                    }
                }
            }
        }

        // 2. leer peticiones del cli
        PaqueteDatos datos;
        ssize_t bytes_leidos = read(tubo_in, &datos, sizeof(PaqueteDatos));

        if (bytes_leidos > 0) {
            RespuestaDatos respuesta;
            memset(&respuesta, 0, sizeof(RespuestaDatos));

            if (datos.orden == 1) { // start
                pid_t pid_hijo = fork();
                if (pid_hijo == 0) {
                    setpgid(0, 0); // para poder matarlo con kill despues
                    execl(datos.ruta_programa, datos.ruta_programa, NULL);
                    exit(1);
                } else if (pid_hijo > 0) {
                    Proceso p;
                    p.iid = siguiente_iid++;
                    p.pid = pid_hijo;
                    p.estado = RUNNING;
                    p.tiempo_inicio = time(NULL);
                    p.reinicios = 0;
                    strcpy(p.ruta, datos.ruta_programa);
                    lista_procesos.push_back(p);

                    respuesta.iid_asignado = p.iid;
                    snprintf(respuesta.mensaje, sizeof(respuesta.mensaje), "IID: %d", p.iid);
                }
            } 
            else if (datos.orden == 2) { // stop
                for (auto &p : lista_procesos) {
                    if (p.iid == datos.id_interno) {
                        kill(p.pid, SIGTERM);
                        p.estado = STOPPED;
                        snprintf(respuesta.mensaje, sizeof(respuesta.mensaje), "Proceso %d detenido", p.iid);
                        break;
                    }
                }
            }
            else if (datos.orden == 3) { // ps
                string salida = "IID\tPID\tSTATE\t\tUPTIME\tBINARY\n";
                for (auto &p : lista_procesos) {
                    long uptime = time(NULL) - p.tiempo_inicio;
                    salida += to_string(p.iid) + "\t" + to_string(p.pid) + "\t" + 
                              estadoToString(p.estado) + "\t\t" + to_string(uptime) + "s\t" + string(p.ruta) + "\n";
                }
                strncpy(respuesta.mensaje, salida.c_str(), sizeof(respuesta.mensaje) - 1);
            }
            else if (datos.orden == 4) { // status
                bool encontrado = false;
                for (auto &p : lista_procesos) {
                    if (p.iid == datos.id_interno) {
                        long uptime = time(NULL) - p.tiempo_inicio;
                        snprintf(respuesta.mensaje, sizeof(respuesta.mensaje), 
                                 "IID:\n%d\nPID:\n%d\nBINARY:\n%s\nSTATE:\n%s\nUPTIME:\n%lds\nRESTARTS:\n%d", 
                                 p.iid, p.pid, p.ruta, estadoToString(p.estado).c_str(), uptime, p.reinicios);
                        encontrado = true;
                        break;
                    }
                }
                if (!encontrado) {
                    snprintf(respuesta.mensaje, sizeof(respuesta.mensaje), "Error: IID %d no encontrado", datos.id_interno);
                }
            }
            else if (datos.orden == 5) { // kill
                for (auto &p : lista_procesos) {
                    if (p.iid == datos.id_interno) {
                        kill(-p.pid, SIGKILL); // mata todo el arbol
                        p.estado = ZOMBIE;
                        snprintf(respuesta.mensaje, sizeof(respuesta.mensaje), "Proceso %d asesinado", p.iid);
                        break;
                    }
                }
            }
            else if (datos.orden == 6) { // zombie
                string salida = "Procesos ZOMBIE:\nIID\tPID\tBINARY\n";
                for (auto &p : lista_procesos) {
                    if (p.estado == ZOMBIE) {
                        salida += to_string(p.iid) + "\t" + to_string(p.pid) + "\t" + string(p.ruta) + "\n";
                    }
                }
                strncpy(respuesta.mensaje, salida.c_str(), sizeof(respuesta.mensaje) - 1);
            }

            // mandar respuesta al cliente
            int tubo_out = open(TUBO_OUT, O_WRONLY);
            if (tubo_out != -1) {
                write(tubo_out, &respuesta, sizeof(RespuestaDatos));
                close(tubo_out);
            }
        }

        usleep(100000); // dormir un poco para no quemar cpu
    }

    close(tubo_in);
    close(dummy_fd);
    return 0;
}