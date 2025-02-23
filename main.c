#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

//Proyecto de SO uso de semaforos en un programa concurrente.
//Integrantes:
//Juan Diego Suarez CI:28083693


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