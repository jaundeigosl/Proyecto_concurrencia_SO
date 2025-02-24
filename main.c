#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#define MAX 3

//Proyecto de SO uso de semaforos en un programa concurrente.
//Integrantes:
//Juan Diego Suarez CI:28083693
//Edgar Enmanuel Gutiérrez Flores CI:28505513


// Estructuras de datos
typedef struct Automovil {
    char idAuto;
    int bufferSenal[MAX];   //Arreglo para el buffer de señales
    int contAmonestaciones; //total de amonestaciones
    float multas;           //valor total de cada multa
    int estacionado;        //1 estacionado, 0 no estacionado
    sem_t mutex;            // semaforo para proteger el acceso de los vehiculos
    sem_t buffer_sem;       // semaforo para controlar el espacio del buffer
}Automovil;

int main(){

    //lectura del archivo

    FILE * file;
    file = fopen("entrada.txt","r");

    if(file == NULL){
        fclose(file);
        exit(1);
    }

    int autos, semaforos, centros_de_multas;

    fscanf(file,"%i",&autos);
    fscanf(file,"%i",&semaforos);
    fscanf(file,"%i",&centros_de_multas);

    printf("autos:%i\n",autos);
    printf("semaforos:%i\n",semaforos);
    printf("centros:%i\n",centros_de_multas);

    fclose(file);

    return 0;
}