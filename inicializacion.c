#include "inicializacion.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Variables globales (definidas en main.c)
extern central centrales[MAX_CENTRALES];
extern semaforo semaforos[MAX_SEMAFOROS];
extern automovil autos[MAX_AUTOS];

void inicializarArrCentrales(int cantCentral) {
    for (int i = 0; i < cantCentral; i++) {
        centrales[i].id = i;
        printf(".........................................\n");
        printf("Identificador de la central en la posicion del arreglo es pos= %d \n", centrales[i].id);
    }
}

void inicializarSemaforos(int cantSemaforo, int cantCentral) {
    int indiceCentral;

    srand(time(NULL));
    if (cantCentral <= MAX_CENTRALES && cantSemaforo <= MAX_SEMAFOROS) {
        for (int i = 0; i < cantSemaforo; i++) {
            semaforos[i].id = i;
            indiceCentral = rand() % cantCentral;
            semaforos[i].centralAsignada = indiceCentral;
            semaforos[i].luzVerde = (rand() % 2);
            semaforos[i].cantAutosAsignados = 0;
            semaforos[i].senalesJustificadas = 0;
            semaforos[i].senalesInjustificadas = 0;

            for (int j = 0; j < MAX_AUTOS; j++) {
                semaforos[i].autosAsignados[j] = 0;
            }

            printf("-----------------------\n");
            printf("Semaforo se le asigno el identificador: %d\n", i);
            printf("Identificador correspondiente a la central del semaforo actual: %d\n", semaforos[i].centralAsignada);
            printf("Estado del semaforo actual: %d\n", semaforos[i].luzVerde);
        }

        printf("\nResumen de Semaforos Asignados:\n");
        for (int j = 0; j < cantSemaforo; j++) {
            printf("-----------------------\n");
            printf("Semaforo S%d\n", semaforos[j].id);
            printf("Central correspondiente: C%d\n", semaforos[j].centralAsignada);
        }
    }
}

void asignacionAutos(int cantAutos, int cantSemaforos) {
    srand(time(NULL));
    for (int i = 0; i < cantAutos; i++) {
        int semaforoAsignado = rand() % cantSemaforos;  // asignacion aleatoria de semaforos
        autos[i].idSemaforoAsignado = semaforoAsignado;
        autos[i].id = i;
        autos[i].tipo = rand() % 2;
        int pos = semaforos[semaforoAsignado].cantAutosAsignados;
        autos[i].pos = pos;
        semaforos[semaforoAsignado].autosAsignados[pos] = i;
        semaforos[semaforoAsignado].cantAutosAsignados++;
        
        // Inicialización de variables del auto
        autos[i].justificadasEmitidas = 0;
        autos[i].justificadasRecibidias = 0;
        autos[i].injustificadasEmitidas = 0;
        autos[i].injustificadasRecibidas = 0;
        autos[i].totalInjustificadasEmitidas = 0;
        autos[i].totalJustificadasRecibidas = 0;
        autos[i].cantBuffer = 0;
        autos[i].indice_entrada = 0;
        autos[i].indice_salida = 0;
        autos[i].amonestaciones = 0;
        autos[i].multas = 0;
        autos[i].multasProcesadas = 0;
        autos[i].senalesPerdidasEmitidas = 0;
        autos[i].senalesPerdidasRecibidas = 0;

        for(int j = 0; j < BUFFER; j++) {
            autos[i].buffer[j].idEmisor = -1;
            autos[i].buffer[j].valida = -1;
        }
        
        printf("Auto %d asignado a Semaforo %d en posicion %d\n", i, semaforoAsignado, pos);
    }
}