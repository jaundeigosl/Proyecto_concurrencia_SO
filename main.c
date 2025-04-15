#include "estructuras.h"
#include "inicializacion.h"
#include "procesos.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

// Variables globales
central centrales[MAX_CENTRALES];
semaforo semaforos[MAX_SEMAFOROS];
automovil autos[MAX_AUTOS];
registroSistema estadisticas;
time_t tiempoGlobal;
int dias;

// Semaforos globales
sem_t semaforoParaActivar;
sem_t semaforos_senales_semaforos[MAX_SEMAFOROS];
sem_t semaforos_senales_autos[MAX_AUTOS];
sem_t semaforos_multas_centrales[MAX_CENTRALES];
pthread_mutex_t mutex_semaforos;
pthread_mutex_t mutex_autos;
pthread_mutex_t mutex_centrales;
pthread_mutex_t mutex_registro;
pthread_mutex_t mutex_dias;

int main() {
    // Inicializar semaforos globales para la sincronización
    sem_init(&semaforoParaActivar, 0, 0);
    pthread_mutex_init(&mutex_autos, NULL);
    pthread_mutex_init(&mutex_semaforos, NULL);
    pthread_mutex_init(&mutex_centrales, NULL);
    pthread_mutex_init(&mutex_registro, NULL);
    pthread_mutex_init(&mutex_dias, NULL);

    // Declaración de hilos
    pthread_t hilosSemaforos[MAX_SEMAFOROS];
    pthread_t hilosAutos[MAX_AUTOS];
    pthread_t hilosCentrales[MAX_CENTRALES];

    // Inicialización del registro del sistema (estadísticas)
    estadisticas.totalSenales = 0;
    estadisticas.totalMultas = 0;
    estadisticas.totalMultasProcesadas = 0;
    estadisticas.totalAmonestaciones = 0;
    estadisticas.totalSenalesPerdidas = 0;

    // Lectura de datos
    FILE *archivo = fopen("entrada.txt", "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    int cantSemaforos, cantCentrales, cantAutos;
    char aux[100]; 

    fscanf(archivo, "%s %d", aux, &cantSemaforos);
    fscanf(archivo, "%s %d", aux, &cantCentrales);
    fscanf(archivo, "%s %d", aux, &cantAutos);

    fclose(archivo);

    // Inicialización de semáforos
    for(int i = 0; i < cantSemaforos; i++) {
        sem_init(&semaforos_senales_semaforos[i], 0, 0);
    }
    for(int i = 0; i < cantAutos; i++) {
        sem_init(&semaforos_senales_autos[i], 0, 0);
    }
    for(int i = 0; i < cantCentrales; i++) {
        sem_init(&semaforos_multas_centrales[i], 0, 0);
    }

    // Inicializar datos para el mapa
    registroMapa datosMapa;
    datosMapa.cantidadSemaforo = cantSemaforos;
    datosMapa.cantidadCentral = cantCentrales;
    datosMapa.cantAutos = cantAutos;

    // Crear hilo del mapa
    pthread_t hiloMapa;
    pthread_create(&hiloMapa, NULL, generarMapa, &datosMapa);

    sleep(1);   // Espera para que se inicialice todo antes de crear los hilos

    for(int i = 0; i < cantSemaforos; i++) {
        pthread_create(&hilosSemaforos[i], NULL, proceso_semaforo, &semaforos[i]);
    }

    for(int i = 0; i < cantCentrales; i++) {
        pthread_create(&hilosCentrales[i], NULL, proceso_central, &centrales[i]);
    }

    dias = 1;
    tiempoGlobal = time(NULL);
    sem_post(&semaforoParaActivar);  // Despertar hilo para terminar

    for(int i = 0; i < cantAutos; i++) {
        if(autos[i].tipo == 1) {
            pthread_create(&hilosAutos[i], NULL, proceso_consumidor, &autos[i]);
        } else {
            pthread_create(&hilosAutos[i], NULL, proceso_emisor, &autos[i]);
        }
    }

    // Esperar a que termine el hilo
    pthread_join(hiloMapa, NULL);

    sleep(1);

    // Cancelar todos los hilos
    for(int i = 0; i < cantAutos; i++) {
        pthread_cancel(hilosAutos[i]);
    }
    for(int i = 0; i < cantSemaforos; i++) {
        pthread_cancel(hilosSemaforos[i]);
    }
    for(int i = 0; i < cantCentrales; i++) {
        pthread_cancel(hilosCentrales[i]);
    }

    // Limpiar semáforos globales
    sem_destroy(&semaforoParaActivar);
    pthread_mutex_destroy(&mutex_autos);
    pthread_mutex_destroy(&mutex_centrales);
    pthread_mutex_destroy(&mutex_semaforos);
    pthread_mutex_destroy(&mutex_dias);
    pthread_mutex_destroy(&mutex_registro);

    // Mostrar estadísticas
    printf("Simulación terminada correctamente\n");

    printf("\n========= Estadísticas del Mes =========\n\n");
    printf("Total de señales: %d\n", estadisticas.totalSenales);
    printf("Total de multas: %d\n", estadisticas.totalMultas);
    printf("Total de amonestaciones: %d\n", estadisticas.totalAmonestaciones);
    printf("Total de señales perdidas: %d\n", estadisticas.totalSenalesPerdidas);
    printf("Total de multas Procesadas: %d\n", estadisticas.totalMultasProcesadas);

    printf("\n========= Estadísticas de Semáforos =========\n\n");
    for(int i = 0; i < cantSemaforos; i++) {
        printf("Semáforo %d:\n", semaforos[i].id);
        printf("Total de señales validadas: %d\n", semaforos[i].senalesJustificadas);
        printf("Total de señales invalidadas: %d\n\n", semaforos[i].senalesInjustificadas);
    }

    printf("\n========= Estadísticas de Centrales =========\n\n");
    for(int i = 0; i < cantCentrales; i++) {
        printf("La central %d:\n", centrales[i].id);
        printf("Cantidad de multas procesadas: %d\n\n", centrales[i].cantMultasProcesadas);
    }

    printf("\n========= Estadísticas de Autos =========\n\n");
    for(int i = 0; i < cantAutos; i++) {
        if(autos[i].tipo) {
            printf("Auto (Consumidor) %d:\n", autos[i].id);
            printf("Total de señales justificadas recibidas: %d\n", autos[i].totalJustificadasRecibidas);
            printf("Total de señales injustificadas recibidas: %d\n", autos[i].injustificadasRecibidas);
            printf("Total de multas pagadas: %d\n", autos[i].multasProcesadas);
            printf("Total señales recibidas perdidas: %d\n\n", autos[i].senalesPerdidasRecibidas);
        } else {
            printf("Auto (Emisor) %d:\n", autos[i].id);
            printf("Total de señales injustificadas recibidas: %d\n", autos[i].injustificadasRecibidas);
            printf("Total de señales justificadas emitidas: %d\n", autos[i].justificadasEmitidas);
            printf("Total de señales injustificadas emitidas: %d\n", autos[i].totalInjustificadasEmitidas);
            printf("Total de multas pagadas: %d\n", autos[i].multasProcesadas);
            printf("Total de señales emitidas perdidas: %d\n\n", autos[i].senalesPerdidasEmitidas);
        }
    }
    
    return 0;
}