#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#define MAX 5

int buffer[MAX];

sem_t mutex, lleno, vacio;
int salida, entrada,contador;


void * automovil(){
    //consumidor -> 0, emisor -> 1, pagar multa -> 2
    srand(time(NULL));
    int item;
    int estado = rand() % (2);

    printf("El estado es %d\n",estado);

    while(1){

        //modo consumidor
        if(estado == 0){

            sem_wait(&lleno);
            sem_wait(&mutex);       
            item = buffer[salida];
            buffer[salida] = 0;
            salida = (salida +1) % MAX;
            contador = contador - 1;
            printf("se consumio y quedan %d elementos\n",contador);
            sem_post(&mutex);
            sem_post(&vacio);

        }else{
            //modo emisor
            if( estado == 1){

                sem_wait(&vacio);
                sem_wait(&mutex);
                buffer[entrada ] = 1;
                entrada = (entrada + 1) % MAX;
                contador = contador + 1;
                printf("se produjo un elemento, ahora hay %d elementos\n",contador);
                sem_post(&mutex);
                sem_post(&lleno);

                //modo pagar multa
            }

        }

    }

}



int main(){

    pthread_t autos[10];

    salida = 0;
    entrada = 0;
    contador = 0;

    sem_init(&mutex,0,1);
    sem_init(&vacio,0,MAX);
    sem_init(&lleno,0,0);

    for(int i = 0; i <5 ; i++){
        buffer[i] = 0;
    }

    for(int i = 0; i <10; i++){
        pthread_create(&autos[i],NULL,&automovil,NULL);
    }

    for(int i = 0; i < 10; i++){
        pthread_join(autos[i],NULL);

    }


    return 0;
}
