#ifndef PROCESOS_H
#define PROCESOS_H

#include "estructuras.h"

// Prototipos de funciones de procesos
void* generarMapa(void* arg);
void* proceso_central(void* arg);
void* proceso_emisor(void* arg);
void* proceso_consumidor(void* arg);
void* proceso_semaforo(void* arg);
int revisarBuffer(int idAutoEmisor, int idAutoReceptor);

#endif