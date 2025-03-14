/*  asignatura: sistemas operativos
    tema: sincronizacion de procesos
    problema 1 con esquema productor consumidor
    estudiantes: 
        Edgar Gutierrez Ci: 28505513
        Juan Suarez Ci: 28083693
        Yosismar Arcia Ci: 27894120
    profesora: Mirella Herrera
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

//CONSTANTES DEFINIDAS 
#define MAX_CENTRALES 1000     		// Maximo de centrales
#define MAX_SEMAFOROS 1000    		// Maximo 50 semaforos
#define MIN 50              		// Minimo valor para la ubicacion
#define MAX 100            			// Maximo valor para la ubicacion
#define MAX_AUTOS 1000				// Maximo de autos 
#define BUFFER 3          			// Maximo de señales que debe reproducir el auto despues de estar en el buffer
#define MAX_ADYACENTES MAX_AUTOS-1	// Maximo de autos adyacentes por emisor
#define N 50   						// Longitud limitada para el buffer de señales 

/*_____________________________________________________________ESTRUCTURAS DEFINIDAS_________________________________________________________________________*/

// ESTRUCTURA PARA LAS SENALES 
typedef struct{
    int idEmisor;   						//identifica al auto emisor que envia la senal
    int contador                            //contador de la misma senal
}senal;

// ESTRUCTURA QUE DEFINE LOS AUTOMOVILES
typedef struct {
    int idAuto;								//identificador del auto
    int tipoVehiculo;                   	//si es emisor, o receptor
	int adyacentes[MAX_AUTOS];         		//contiene el identificador de los autos adyacentes a un emisor
    int cantAdy;							//cantidad de autos adyacentes
 	int contAmonestaciones;            		//total de amonestaciones
    int multas;                       		//valor total de cada multa
    int justificadas;                		//total de senales justificadas
    int injustifcada;						//total de senales injustificadas
    int estacionado;                		//1 estacionado, 0 no estacionado
    senal * bufferSenales[MAX];     			//Arreglo de punteros para el buffer de senales
    int cantSenales;
    int entrada;
    int salida;
}automovil;

// Estructura para representar una central
typedef struct {
    int id;            						//identificador y ubicacion de la central
} centrales;

// Estructura para representar un semaforo
typedef struct {
    int idSem;                      		//identificador de los semaforos 
    int luzVerde;                  			//para indicar el estado del semaforo 
    centrales centralAsignada;    			//centrales asignadas por semaforo    
}semaforos;

//variables globales 
sem_t semafore[MAX_AUTOS];	//Declara un arreglo de semaforos que estara en paralelo con el arreglo de automoviles 
sem_t mutex;    			//Declara un semaforo llamado mutex e inicializa. Se utilizan para proteger la seccion critica, asegurando que solo un hilo a la vez pueda acceder a el recurso compartido.
sem_t lleno;    			//Declara un semaforo de comunicacion llamado lleno e inicializa. Se utiliza para señalar cuando el bufer esta lleno.
sem_t vacio;    			//Declara un semaforo de comunicacion llamado vacio e inicializa. Se utiliza para señalar cuando el bufer esta vacío.

automovil listaAutos[MAX_AUTOS];					//arreglo de automoviles
centrales listaCentrales[MAX_CENTRALES];			//arreglo de centrales del mapa		
semaforos semaforo_arr[MAX_SEMAFOROS];				//arreglo de semaforos del mapa

//procedimientos para inicializar semaforos, senales, automoviles y centrales 
void inicializarArrCentrales(int cantidadCentral)
{
	int i;
	
    // Inicializa la informacion de las centrales
    for (i = 0; i < cantidadCentral; i++) {							//para recorrer el arreglo de centrales
        listaCentrales[i].id = i;                                   //para asignar identificadores de centrales 
        
        /*______________________BANDERAS__________________*/
        printf(".........................................\n");
        printf("identificador de la central en la posicion del arreglo es pos= %d y id=%d",i,listaCentrales[i].id);
        printf("\n");
    }
}

void inicializarSemaforos(int cantidadSemaforo, int cantidadCentral) {
    int indiceCentral,i,j;
   
    srand(time(NULL));						//para garantizar que la generacion de valores aleatorios sean diferentes segun el tiempo en que se ejecute el programa
     
    //verifica que la cantidad de centrales y de semaforos se encuentre entre la cantidad maxima indicada para cada arreglo 
    if(cantidadCentral <= MAX_CENTRALES && cantidadSemaforo <= MAX_SEMAFOROS)
	{
        // Asignar centrales a los semaforos
        for( i = 0; i < cantidadSemaforo; i++){											//Para recorrer el arreglo de semaforos
            semaforo_arr[i].idSem = i;                     								//Para asignar el identidador al semaforo 
            indiceCentral = rand() % cantidadCentral;      								//Para seleccionar una central aleatoria
            semaforo_arr[i].centralAsignada = listaCentrales[indiceCentral]; 				//Para asignar la central
            semaforo_arr[i].luzVerde = (rand() % 2);         								//Para asignar el estado del semaforo
             
            /*________________________________BANDERAS__________________________________________*/
            printf("-----------------------\n");
            printf("Semaforo Se le asigno el identificador: %d \n", i);
            printf("indice generado que corresponde al arreglo de centrales: %d \n",indiceCentral);
            printf("identificador correspondiente a la central del semaforo actual: %d \n",semaforo_arr[i].centralAsignada.id);
            printf("estado del semaforo actual: %d \n", semaforo_arr[i].luzVerde);
        }
 		printf("\n");
        // Imprime la  informacion de los semaforos
        for(j = 0; j < cantidadSemaforo; j++) {
            printf("-----------------------\n");
            printf("Semaforo S%d \n", semaforo_arr[j].idSem);
            printf("Central correspondiente: C%d \n",semaforo_arr[j].centralAsignada.id);
        }
    }
 }

void inicializarAutos(int cantAutos, int maxAdyacentes)
{
	srand(time(NULL));							//para garantizar que la generacion de valores aleatorios sean diferentes segun el tiempo en que se ejecute el programa
	int tipo, i, j, k, adyacenteId, posicionAdyacente, adyacentesAsignados=0, asignado;	

    //decidir que tipo de auto es cada uno	
    for(i=0;i<cantAutos;i++)
    {
		tipo = (rand()%2)+1;	//optiene el valor aleatorio que indica que tipo de automovil se va a crear
		//Para asignar valores al vehiculo segun e tipo de auto que se indique en la variable tipo
		switch (tipo){
        case 1:
            //EMISOR
            	listaAutos[i].idAuto=i+1;				//identificador del auto
            	listaAutos[i].tipoVehiculo = tipo;		//tipo emisor
                listaAutos[i].cantAdy = 0;				//cantidad de adyacentes al inicio del programa
        break;
        case 2:
            //RECEPTOR
            	listaAutos[i].idAuto=i+1;				//identificador del auto 
            	listaAutos[i].tipoVehiculo = tipo;		//tipo receptor
                listaAutos[i].entrada = 0;              //indice del buffer para el consumidor
                listaAutos[i].salida = 0;               //indice del buffer para el productor

                for(int j = 0; j < MAX; j++){
                    listaAutos[i].bufferSenales[j] = NULL;  //Inicializando el buffer de senales
                }

        break; 
        default:
            printf("Opcion invalida. Por favor, selecciona una opcion valida.\n");
        break;
    	}
	}
    
    // Asignar adyacentes a los autos emisores
    for (i = 0; i < cantAutos; i++) {            							//recorre el arreglo de autos 
        if (listaAutos[i].tipoVehiculo == 1){  								// Si es emisor
            adyacentesAsignados = 0;										// valor actual de adyacentes asignados se inicializa en 0
            while(adyacentesAsignados < maxAdyacentes){						// para controlar que la cantidad de adyacentes no  se pase de lo indicado
                posicionAdyacente = rand() % cantAutos;    					//genera posiciones aleatorias que corresponden al arreglo de autos 
                // Asegurar que sea diferente y receptor														
                if (posicionAdyacente != i && listaAutos[posicionAdyacente].tipoVehiculo == 2)
				{ 
                    adyacenteId = listaAutos[posicionAdyacente].idAuto; 	//obtengo el ID del adyacente a traves del vector de autos 
                    // Verificar si el adyacente ya fue asignado
                    asignado = 0;
                    for (k = 0; k < adyacentesAsignados; k++)				//para recorrer el arreglo de adyacentes de cada auto indicado 
					{	
						//para ver si este adyacente ya se encuentra en el arreglo
                        if (listaAutos[i].adyacentes[k] == adyacenteId)
						{ 
                            asignado = 1;    								//cambia la bandera 
                            break;
                        }
                    }
                    if (!asignado)
					{   //si no fue asignado, entonces inserto el identificador del auto escogido e incremento las variables contadoras
                        listaAutos[i].adyacentes[adyacentesAsignados] = adyacenteId;
                        adyacentesAsignados++;
                        listaAutos[i].cantAdy++;
                    }
                }
            }
        }
    }
    
    // Imprimir informacion de los autos
    for(i=0;i<cantAutos;i++)
    {
        if(listaAutos[i].tipoVehiculo == 1){
            printf("auto: %d es emisor \n",listaAutos[i].idAuto);
            printf("Adyacentes: ");
            for (j = 0; j < listaAutos[i].cantAdy; j++) 
			{
                printf("%d ", listaAutos[i].adyacentes[j]);
            }
            printf("\n");
        }else{
            printf("auto: %d es receptor \n",listaAutos[i].idAuto);
        }
    }
}


 //PROCESOS 
//proceso que genera el mapa donde se ubican las centrales y semaforos
void generadorMapa(int cantidadSemaforo, int cantidadCentral)
{
    inicializarArrCentrales(cantidadCentral);
    inicializarSemaforos(cantidadSemaforo,cantidadCentral);
}

//procedimientos para los procesos emisor y receptor 
senal *producir(int id) {
    senal *aux = (senal *)malloc(sizeof(senal));    //creando la senal
    aux->idEmisor = id;                             //asignandole el id del productor
    aux->contador = 1;                              //cantidad de senales actualmente del mismo emisor
    return 	aux;    		
}

void consumir(int item) {
    printf("Senal consumida: %d \n", item);			//para procesar la senal que se consume

}

//proceso en funcion del auto emisor
void *emisor(void *arg)
{
    int dia=1;   													//variable local dia para controlar que tanto debe ejecutarse los hilos
    int encontrado;                                             //variable que indica si se encuentra ya una senal en el buffer
	int indice_buffer;                                        //indica la cantidad de autos adyacentes recorridos
    struct automovil *autoEmisor = (struct automovil *) arg;			//desempaquetado de la estructura
	senal * senalProducida = NULL;  
    senal * buscador = NULL;                                           
	//pthread_t id = pthread_self();
    //printf("ID del hilo: %lu\n", (unsigned long)id);
	//printf("%i \n",idDesenmascarada->idhilo);
	                                       
    while (dia<=7){                    									//bucle para que el emisor envia senales (puede ser infinito)
    	printf("dia %d \n",dia);
    	
        sem_wait(&vacio);    											//semaforo que comunica
        sem_wait(&mutex);        										//semaforo que sincroniza este bloquea el acceso a la seccion crtica mientras el emisor hace uso de ella
        
		/*_________________________Seccion critica________________*/ 

        encontrado = 0;
        indice_buffer = 0;

        for(int i = 0; i < autoEmisor->cantAdy){    //iterando sobre los adyacentes
            encontrado = 0;
            indice_buffer = 0;
            while(!encontrado && (indice_buffer < MAX)){    //iterando sobre el buffer buscando si ya habia una senal del emisor
                buscador = listaAutos[autoEmisor->adyacentes[i]].bufferSenales[indice_buffer];
                if(buscador != NULL){
                    if(buscador->idEmisor == autoEmisor->idAuto){   //se verifica si la senal fue hecha por el actual emisor
                        encontrado = 1;
                        if(buscador->idEmisor < 3){
                            buscador->idEmisor = buscador->idEmisor + 1;
                        }else{
                            //maximo de senales alcanzadas
                        }
                    }
                }
                indice_buffer++;
            }
            if(!encontrado && listaAutos[adyacentes[i]].cantSenales < MAX ){//si no se encontro y hay espacio entonces se agrega al buffer
                senalProducida = producir(autoEmisor->idAuto);   //la funcion producir devolvera la senal que se debe ingresar al bufer
                listaAutos[adyacentes[i]].bufferSenales[listaAutos[adyacentes[i]].entrada] = senalProducida;    //agregando la senal al buffer del receptor
                listaAutos[adyacentes[i]].entrada = (listaAutos[adyacentes[i]].entrada + 1) % MAX_ADYACENTES;   //aumentando el indice del buffer para ingresar senales
                listaAutos[adyacentes[i]].cantSenales++ ;

            }
            buscador = NULL;
        }

        senalProducida = NULL;
		        
        /*_____________________FIN SECCION CRITICA_______________*/
        sem_post(&mutex);												//informa que esta desbloqueado
        sem_post(&lleno);												//informa que se lleno
    	dia++;
    	printf("***********************************************\n");
    }  
    printf("\n\n");  
    pthread_exit(0);													//termina la ejecucion del hilo emisor
}

//proceso en funcion del auto receptor
void *receptor(void *arg)
{
    int item,dia=1;    													//variable local item para almacenar el elemento consumido.
    
	struct automovil *autoReceptor = (struct automovil *) arg;			//desempaquetado de la estructura
	
    senal * aux;
	//pthread_t id = pthread_self();
    //printf("ID del hilo: %lu\n", (unsigned long)id);
	//printf("%i \n",idDesenmascarada->idhilo);
	
    while (dia<=7) {    												//bucle para que el consumidor siga consumiendo elementos.
    	printf("dia %d \n",dia);
    	
        sem_wait(&lleno);												//semaforo que comunica 
        sem_wait(&mutex);    											//semaforo que sincroniza y bloquea la seccion critica del receptor
        
		/*_________________SECCION CRITICA___________________________*/
		
        aux = autoReceptor->bufferSenales[autoReceptor->salida];
        /*
            Validar senal
        */

        //suponiendo es valida

        //consumiendo la senal, aumentando la salida y disminuyendo la cantidad de senales
        autoReceptor->bufferSenales[autoReceptor->salida] = NULL;
        autoReceptor->salida = (autoReceptor->salida + 1) % MAX_ADYACENTES;
        autoReceptor->cantSenales = autoReceptor->cantSenales - 1 ;


        /*_____________________FIN SECCION CRITICA_______________*/

        sem_post(&mutex);
        sem_post(&vacio);
        consumir(item);    												//funcion que procesa la senal consumida que se indica en item
     	dia++;
     	printf("........................................................................\n");
    }
    printf("\n\n");
    pthread_exit(0);

/*
-. sem_wait(lleno) Llama a la función sem_wait con el semáforo lleno. Esta llamada decrementa 
el valor del semáforo lleno. Si el valor del semáforo es cero (lo que indica que el búfer está 
vacío), el hilo se bloqueará hasta que el semáforo se incremente (lo que indica que hay un 
elemento disponible en el búfer).

-. sem_wait(mutex) Llama a la función sem_wait con el semáforo mutex. Esta llamada decrementa
el valor del semáforo mutex. El semáforo mutex se utiliza para proteger la sección crítica
del código, asegurando que solo un hilo a la vez pueda acceder al búfer compartido. Si el
valor del semáforo es cero (lo que indica que otro hilo está accediendo al búfer), el hilo 
se bloqueará hasta que el semáforo se incremente.
*/

}
//proceso en funcion de tratamiento de multas


//proceso en funcion de los semaforos


//proceso en funcion de central de multas




//PROGRAMA PRINCIPAL
int main(void){
    int cSemaforos = 10;
    int cCentrales = 10;
    int autos = 5;
    int adyacentes=4;
    int i,k;
    //Declara una variable para almacenar los atributos de los hilos.
    pthread_attr_t atrib;
    //Declara dos variables para almacenar los identificadores de los hilos productor y consumidor.
    pthread_t hiloconsumidor, hiloproductor[4];
    
    /*Llamada a procedimientos que permiten inicializar datos necesarios para comenzar el programa*/
    printf("INICIALIZANDO AUTOS\n");
    inicializarAutos(autos, adyacentes);
    printf("GENERANDO EL MAPA\n");
    generadorMapa(cSemaforos,cCentrales);
    printf("\n\n");
    //inicialización atributos de los hilos con valores determinados 
    pthread_attr_init(&atrib);        
    entrada= 0; salida= 0; contador= 0;
    
    sem_init(&mutex, 0, 1);
    sem_init(&vacio, 0, N);
    sem_init(&lleno, 0, 0);

    //creación de hilos y se ejecutan en funcion de si es emisor o receptor 
    for( i=0;i<4;i++){
        pthread_create(&hiloproductor[i], &atrib, emisor, NULL);
    }
    pthread_create(&hiloconsumidor, &atrib, receptor, NULL);
    
    //para esperar que un hilo en particular termine 
    for(i=0;i<4;i++){
        pthread_join(hiloproductor[i], NULL);
    }    
    pthread_join(hiloconsumidor, NULL);
    
    printf("contenido final del buffer\n");
    for( k=0;k<N;k++)
    {
       printf ("%d - ",buffer[k]);
    }
    return 0;
}