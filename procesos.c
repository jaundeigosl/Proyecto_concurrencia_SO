#include "procesos.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include "inicializacion.h"

// Variables globales (definidas en main.c)
extern central centrales[MAX_CENTRALES];
extern semaforo semaforos[MAX_SEMAFOROS];
extern automovil autos[MAX_AUTOS];
extern registroSistema estadisticas;
extern time_t tiempoGlobal;
extern int dias;

// Semaforos globales (definidos en main.c)
extern sem_t semaforoParaActivar;
extern sem_t semaforos_senales_semaforos[MAX_SEMAFOROS];
extern sem_t semaforos_senales_autos[MAX_AUTOS];
extern sem_t semaforos_multas_centrales[MAX_CENTRALES];
extern pthread_mutex_t mutex_semaforos;
extern pthread_mutex_t mutex_autos;
extern pthread_mutex_t mutex_centrales;
extern pthread_mutex_t mutex_registro;
extern pthread_mutex_t mutex_dias;

int revisarBuffer(int idAutoEmisor, int idAutoReceptor) {
    int cantidad = autos[idAutoReceptor].cantBuffer;
    int iterador = autos[idAutoReceptor].indice_salida;
    int contadorSenales = 0;
    int contadorEspacios = 0;
    int respuesta = 0;

    while(contadorEspacios < cantidad && contadorSenales < 3) {
        if(autos[idAutoReceptor].buffer[iterador].idEmisor == idAutoEmisor) {
            contadorSenales++;
        }   
        contadorEspacios++;
        iterador = (iterador + 1) % BUFFER;
    }

    if(contadorSenales < 3) {
        respuesta = 1;
    }

    return respuesta;
}

void* generarMapa(void* arg) {
    registroMapa* datosMapa = (registroMapa*)arg;   

    pthread_mutex_lock(&mutex_autos);
    pthread_mutex_lock(&mutex_semaforos);
    pthread_mutex_lock(&mutex_centrales);

    // Inicializar estructuras
    inicializarArrCentrales(datosMapa->cantidadCentral);
    inicializarSemaforos(datosMapa->cantidadSemaforo, datosMapa->cantidadCentral);
    
    if (datosMapa->cantAutos > 0) {
        asignacionAutos(datosMapa->cantAutos, datosMapa->cantidadSemaforo);
    }

    pthread_mutex_unlock(&mutex_centrales);
    pthread_mutex_unlock(&mutex_semaforos);
    pthread_mutex_unlock(&mutex_autos);

    sem_wait(&semaforoParaActivar);

    while (true) {
        if(dias > MES) {
            break;
        }

        if((time(NULL) - tiempoGlobal) % DIA == 0) {
            pthread_mutex_lock(&mutex_dias);
            printf("\n======= DIA %d =======\n\n", dias);
            dias++;
            pthread_mutex_unlock(&mutex_dias);
            sleep(1);
        }
    }
        
    return NULL;
}

void* proceso_emisor(void* arg) {
    automovil* autoProceso = (automovil*) arg;
    int adyacentes = semaforos[autoProceso->idSemaforoAsignado].cantAutosAsignados - 1;
    int iterador, adyacentesRecorridos, idAdyacente;

    pthread_mutex_lock(&mutex_autos);
    printf("\n Hilo %lu esta procesando el Auto (emisor) %d\n", pthread_self(), autoProceso->id);
    autoProceso->diaInicio = dias;
    pthread_mutex_unlock(&mutex_autos);

    while(1) {
        iterador = 0;
        adyacentesRecorridos = 0;
        
        while(adyacentesRecorridos < adyacentes) {
            pthread_mutex_lock(&mutex_autos);
            pthread_mutex_lock(&mutex_registro);

            if(iterador != autoProceso->pos) {
                idAdyacente = semaforos[autoProceso->idSemaforoAsignado].autosAsignados[iterador];
                adyacentesRecorridos++;

                if((autos[idAdyacente].cantBuffer == BUFFER) || !revisarBuffer(autoProceso->id, idAdyacente)) {
                    printf("Se pierde una senal del auto %d enviada al auto %d \n", autoProceso->id, idAdyacente);
                    estadisticas.totalSenalesPerdidas++;
                    autoProceso->senalesPerdidasEmitidas++;
                    autos[idAdyacente].senalesPerdidasRecibidas++;
                } else {
                    printf("Se envia una senal del auto %d al auto %d \n", autoProceso->id, idAdyacente);
                    autos[idAdyacente].buffer[autos[idAdyacente].indice_entrada].idEmisor = autoProceso->id;
                    autos[idAdyacente].buffer[autos[idAdyacente].indice_entrada].valida = 0;
                    autos[idAdyacente].cantBuffer++;
                    autos[idAdyacente].indice_entrada = (autos[idAdyacente].indice_entrada + 1) % BUFFER;
                }

                estadisticas.totalSenales++;
            }

            iterador++;
            pthread_mutex_unlock(&mutex_registro);
            pthread_mutex_unlock(&mutex_autos);
            usleep(1000);
        }

        sem_post(&semaforos_senales_semaforos[autoProceso->idSemaforoAsignado]);

        // Revisando si tiene multas y pagandolas
        pthread_mutex_lock(&mutex_autos);
        pthread_mutex_lock(&mutex_registro);

        if(autoProceso->multas > 0) {
            printf("El auto %d (emisor) pago una multa en la central %d\n", autoProceso->id, semaforos[autoProceso->idSemaforoAsignado].centralAsignada);
            
            autoProceso->multas--;
            autoProceso->multasProcesadas++;
            estadisticas.totalMultasProcesadas++;
            sem_post(&semaforos_multas_centrales[semaforos[autoProceso->idSemaforoAsignado].centralAsignada]);
        }

        pthread_mutex_unlock(&mutex_registro);
        pthread_mutex_unlock(&mutex_autos);
    }
}

void* proceso_consumidor(void* arg) {
    automovil* autoProceso = (automovil*) arg;

    pthread_mutex_lock(&mutex_autos);
    printf("\n Hilo %lu esta procesando el Auto (receptor) %d\n", pthread_self(), autoProceso->id);
    autoProceso->diaInicio = dias;
    pthread_mutex_unlock(&mutex_autos);

    while(1) {
        sem_wait(&semaforos_senales_autos[autoProceso->id]);

        pthread_mutex_lock(&mutex_autos);
        for(int i = 0; i < BUFFER; i++) {
            if(autoProceso->buffer[i].valida == 1) {
                printf("Auto %d consume una senal del auto %d\n", autoProceso->id, autos[autoProceso->buffer[i].idEmisor].id);
                autoProceso->buffer[i].valida = -1;
                autoProceso->buffer[i].idEmisor = -1;
                autoProceso->cantBuffer--;
            }
        }
        pthread_mutex_unlock(&mutex_autos);

        // Revisando si tiene multas y pagandolas
        pthread_mutex_lock(&mutex_autos);
        pthread_mutex_lock(&mutex_registro);

        if(autoProceso->multas > 0) {
            printf("El auto %d (consumidor) pago una multa en la central %d\n", autoProceso->id, semaforos[autoProceso->idSemaforoAsignado].centralAsignada);
            
            autoProceso->multas--;
            autoProceso->multasProcesadas++;
            estadisticas.totalMultasProcesadas++;
            sem_post(&semaforos_multas_centrales[semaforos[autoProceso->idSemaforoAsignado].centralAsignada]);
        }

        pthread_mutex_unlock(&mutex_registro);
        pthread_mutex_unlock(&mutex_autos);
    }
}

void* proceso_semaforo(void* arg) {
    semaforo* semaforoProceso = (semaforo*) arg;
    int dia, diaActual, autoValidando, autoEmisor;

    pthread_mutex_lock(&mutex_semaforos);
    printf("\n Hilo %lu esta procesando Semaforo %d\n", pthread_self(), semaforoProceso->id);
    pthread_mutex_unlock(&mutex_semaforos);

    pthread_mutex_lock(&mutex_dias);
    dia = dias;
    pthread_mutex_unlock(&mutex_dias);

    while(1) {
        pthread_mutex_lock(&mutex_dias);
        diaActual = dias;
        pthread_mutex_unlock(&mutex_dias);

        if(diaActual != dia) {
            dia = diaActual;
            pthread_mutex_lock(&mutex_semaforos);
            semaforoProceso->luzVerde = !semaforoProceso->luzVerde;
            printf("Semaforo %d cambia a %s\n", semaforoProceso->id, semaforoProceso->luzVerde ? "VERDE" : "ROJO");
            pthread_mutex_unlock(&mutex_semaforos);
        }

        sem_wait(&semaforos_senales_semaforos[semaforoProceso->id]);

        for(int i = 0; i < semaforoProceso->cantAutosAsignados; i++) {
            autoValidando = semaforoProceso->autosAsignados[i];

            pthread_mutex_lock(&mutex_autos);
            pthread_mutex_lock(&mutex_registro);
            pthread_mutex_lock(&mutex_semaforos);
            pthread_mutex_lock(&mutex_dias);

            autoEmisor = autos[autoValidando].buffer[autos[autoValidando].indice_salida].idEmisor;

            if(autos[autoValidando].buffer[autos[autoValidando].indice_salida].valida == 0) {
                if(autos[autoValidando].tipo && semaforoProceso->luzVerde) {
                    // Proceso de validación de señal
                    semaforoProceso->senalesJustificadas++;
                    autos[autoValidando].justificadasRecibidias++;
                    autos[autoEmisor].justificadasEmitidas++;

                    if(dias - autos[autoValidando].diaInicio <= 7 && autos[autoValidando].justificadasRecibidias >= 200) {
                        printf("El auto %d llego a las %d senales justificadas recibidas en 7 dias o menos, se emite una multa\n", autos[autoValidando].id, autos[autoValidando].justificadasRecibidias);
                        autos[autoValidando].diaInicio = dias;
                        autos[autoValidando].multas++;
                        autos[autoValidando].totalJustificadasRecibidas += autos[autoValidando].justificadasRecibidias;
                        autos[autoValidando].justificadasRecibidias = 0;
                        estadisticas.totalMultas++;
                    } else if(dias - autos[autoValidando].diaInicio > 7) {
                        autos[autoValidando].diaInicio = dias;
                        autos[autoValidando].totalJustificadasRecibidas += autos[autoValidando].justificadasRecibidias;
                        autos[autoValidando].justificadasRecibidias = 0;
                    }

                    autos[autoValidando].buffer[autos[autoValidando].indice_salida].valida = 1;
                    printf("El semaforo %d valido una senal. Auto emisor:%d -> Auto consumidor:%d\n", semaforoProceso->id, autoEmisor, autoValidando);
                    printf("Auto consumidor %d tiene ahora %d/200 senales justificadas recibidas\n", autos[autoValidando].id, autos[autoValidando].justificadasRecibidias);
                } else {
                    // Proceso de invalidación de señal
                    semaforoProceso->senalesInjustificadas++;
                    autos[autoValidando].injustificadasRecibidas++;
                    autos[autoEmisor].injustificadasEmitidas++;
                    estadisticas.totalAmonestaciones++;

                    if(dias - autos[autoEmisor].diaInicio <= 7 && autos[autoEmisor].injustificadasEmitidas >= 150) {
                        printf("El auto %d llego a las %d senales injustificadas emitidas en 7 dias o menos, se emite una multa\n", autos[autoEmisor].id, autos[autoEmisor].injustificadasEmitidas);
                        autos[autoEmisor].diaInicio = dias;
                        autos[autoEmisor].multas++;
                        autos[autoEmisor].totalInjustificadasEmitidas += autos[autoEmisor].injustificadasEmitidas;
                        autos[autoEmisor].injustificadasEmitidas = 0;
                        estadisticas.totalMultas++;
                    } else if(dias - autos[autoEmisor].diaInicio > 7) {
                        autos[autoEmisor].diaInicio = dias;
                        autos[autoEmisor].totalInjustificadasEmitidas += autos[autoValidando].injustificadasEmitidas;
                        autos[autoEmisor].injustificadasEmitidas = 0;
                    }

                    autos[autoValidando].buffer[autos[autoValidando].indice_salida].idEmisor = -1;
                    autos[autoValidando].buffer[autos[autoValidando].indice_salida].valida = -1;
                    autos[autoValidando].cantBuffer--;
                    printf("El semaforo %d invalido una senal. Auto emisor:%d -> Auto consumidor:%d\n", semaforoProceso->id, autoEmisor, autoValidando);
                    printf("Auto emisor %d tiene ahora %d/150 senales injustificadas\n", autos[autoEmisor].id, autos[autoEmisor].injustificadasEmitidas);
                }

                autos[autoValidando].indice_salida = (autos[autoValidando].indice_salida + 1) % BUFFER;
            }

            sem_post(&semaforos_senales_autos[autoValidando]);

            pthread_mutex_unlock(&mutex_dias);
            pthread_mutex_unlock(&mutex_semaforos);
            pthread_mutex_unlock(&mutex_registro);
            pthread_mutex_unlock(&mutex_autos);
        }
    }
}

void* proceso_central(void* arg) {
    central* centralProceso = (central*) arg;

    pthread_mutex_lock(&mutex_centrales);
    printf("\n Hilo %lu esta procesando Central %d\n", pthread_self(), centralProceso->id);
    pthread_mutex_unlock(&mutex_centrales);

    while(1) {
        sem_wait(&semaforos_multas_centrales[centralProceso->id]);
        pthread_mutex_lock(&mutex_centrales);
        centralProceso->cantMultasProcesadas++;
        pthread_mutex_unlock(&mutex_centrales);
    }
}