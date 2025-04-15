#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <stdbool.h>

//constantes
#define MAX_CENTRALES 1000           
#define MAX_SEMAFOROS 10000     
#define RADIO_MAPA 500              
#define MAX_AUTOS 25000
#define BUFFER 10
#define MES 30
#define DIA 2

// estructura para definir las centrales
typedef struct central{
    int id;     //identificador de la central
    int cantMultasProcesadas; //Cantidad de multas procesadas

}central;

// estructura para definir las senales
typedef struct senal{
    int idEmisor;   //Id del emisor
    int valida;     //Indica si es una senal validada
}senal;

// estructura para definir los autos
typedef struct automovil{
    int id;
    int pos;     //Posicion del auto en el arreglo
    int tipo;   //Si es consumidor 1, o si es productor 0
    int idSemaforoAsignado;
    int cantBuffer; //cantidad de senales en el buffer
    senal buffer[BUFFER];   //buffer de senales
    int indice_entrada;  //Posicion en la que se agrega
    int indice_salida;  //Posicion en la que se consume
    int multas; //Total de multas recibidas
    int multasProcesadas;   //Total de multas pagadas
    int amonestaciones; //Senanales injustificadas emitidas en total
    int justificadasRecibidias;
    int injustificadasRecibidas;
    int justificadasEmitidas;
    int injustificadasEmitidas; //Semanalmente, luego se pasan a amonestaciones
    int senalesPerdidasEmitidas;    //Senales que se pierden porque el buffer esta lleno
    int senalesPerdidasRecibidas;    //Senales que se pierden porque el buffer esta lleno
    int diaInicio;     //Indica el dia de inicio del hilo
    int totalJustificadasRecibidas;
    int totalInjustificadasEmitidas;

}automovil;

// estructura para definir los semaforos
typedef struct semaforo{
    int id;  
    int centralAsignada;    //central que controla este semaforo
    int luzVerde;    //si es 0, es rojo y si es 1 es verde
    int cantAutosAsignados; // cantidad de autos que son controlados por el semaforo
    int autosAsignados[MAX_AUTOS];  //arreglo de ids de autos asignados
    int senalesJustificadas;    //total e senales validadas
    int senalesInjustificadas;  //total de senales rechazadas
}semaforo;

// estructura para definir el mapa
typedef struct registroMapa{
    int cantidadSemaforo;
    int cantidadCentral;
    int cantAutos;
} registroMapa;

// estructura para lleva las estadisticas del sistema
typedef struct registroSistema{
    int totalSenales;
    int totalMultas;
    int totalMultasProcesadas;
    int totalAmonestaciones;
    int totalSenalesPerdidas;
}registroSistema;

#endif