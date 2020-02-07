/* ----- SERVIDOR QUE RECIBIRÁ LAS PETICIONES DE CONTRASEÑAS -----

Integrantes: 
- Cristia Alexis Sánchez Montoya
- Cristian Alberto Arenas

Proyecto Sistemas Operativos 2017 - 2
*/

// Librerias necesarias
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include "passwords.h"


#define MAX MAX_LENGTH // Número máximo de carácteres que tendrá la contraseña
#define N_PETICIONES 50 //Número de peticiones que recibirá el objetivo simultaneamente
//#define N_HILOS 100

int esCorrecta(char[]);
void* recibir(void*);
void initPassword();

//pthread_t hilos[N_HILOS]; //Número de hilos 
sem_t semaforo; //Semaforo para el control de las peticiones
char* password = NULL;//Password que se definirá al inicio del programa
int descifrada = 0;//Binario para saber si a sido descifrada
int fileDes1;

/**
 *Reserva memoria para guardar la contraseña
 **/
void initPassword(){
    password = (char*) malloc(sizeof(char) * MAX);
}

/**
 * Función que ejecutarán los hilos para recibir las peticiones de los atacantes
 * Se enviará como parametro el fileDescriptor
 **/
void *recibir(void *fdd){
    
    char passwordRcv[MAX];
    int fileDes = *(int*)fdd; //Se convierte en un entero
    int atacante = *((int*)fdd+1);//El número del atacante
    char *filePath = (char*)malloc(sizeof(char)*50);//Reserva de memoria para ubicación del archivo
    sprintf(filePath, "ataques/objetivos/objetivo%d.lst", getpid());

    //Ciclo que se queda recibiendo los ataques hasta que la contraseña sea descifrada
    while(!descifrada){
        sem_wait(&semaforo);
        if (recv(fileDes,passwordRcv,MAX,0) == -1){
            printf("Error en recv() \n");
            exit(-1);
        }

        passwordRcv[MAX] = '\0';

        if(esCorrecta(passwordRcv)){
            printf("La contraseña '%s' del atacante #%d es CORRECTA\n¡CONTRASEÑA DESCIFRADA!\n\n",passwordRcv,atacante);
            descifrada = 1;
            fflush(stdout);
        }else{
            printf("La contraseña '%s' del atacante #%d es INCORRECTA\n",passwordRcv,atacante);
            fflush(stdout);
        }
        writeFile(filePath,passwordRcv);//Escribe la contraseña que recibió en su archivo
        sem_post(&semaforo);
    }
    //remove(filePath);
    close(fileDes);
    close(fileDes1);
    exit(0);
}

/**
 * Permite saber si una contraseña es correcta
 */
int esCorrecta(char pass[]){
    return (strcmp(password,pass) == 0);
}

/**
 * Permite resetear las carpetas donde se guardan los archivos de los ataques, para crearlas nuevas para cada 
 * ejecución
 */
void resetDir(){
    system("rm -rf ataques");
    system("mkdir -p ataques/atacantes");
    system("mkdir -p ataques/objetivos");
}

// FUNCION PRINCIPAL
int main(int argc,char **argv){

    //Resete los directorios al iniciar
    resetDir();
    
    //Reserva la memoria para la contraseña
    initPassword();
    
    //Instrucciones si el archivo fue mal ejecutado
    if(argc != 4 && argc != 3 && argc != 2){

        printf("\n\n\n\n\n");
        printf("Comando invalido.\n----- INSTRUCCIONES -----\n"
        "Debe enviar dos parametros como máximo, estos son:\n"
        "1. El puerto por el cual se realizará la conexión.\n"
        "2. Una contraseña de maximo %d caracteres (Opcional).\nEjemplo de uso:\n",MAX);
        printf("[./objetivo 1025 password] para definir la contraseña\n");
        printf("[./objetivo 1025 - 3] para generar una contraseña de nivel(1-3)\n");
        printf("[./objetivo 1025] para generar una contraseña aleatoria\n");
        
        printf("\n\n\n\n\n");
        exit(-1);
    }else if(argc == 4){
        //Creando una contraseña aleatoria de un nivel de 1 a 3
        int nivel = atoi(argv[3]);
        if(nivel < 1 || nivel > 3){ //Si el nivel de la contraseña es incorrecto
            printf("Nivel de la contraseña invalido\nNiveles permitidos:\n1 (Bajo)\n2 (Medio)\n3 (Alto)\n");
            exit(-1);
        }
        strcpy(password,getPassword(nivel));

    }else if(argc == 3){

        //Creando una contraseña definida por el usuario
        if(strlen(argv[2]) > MAX){//La contraseña no debe ser mayor al nivel maximo (8 por defecto)
            printf("La contraseña no puede ser de más de %d caracteres\n",MAX);
            exit(-1);
        }
        strcpy(password,argv[2]);
    }else{
        //Si sólo fue enviado un parametro, el programa crea una contraseña aleatoria de la lista
        strcpy(password,getPassword(1));
    }

    // declarando Semaforo
    int sem_error = sem_init(&semaforo,0,N_PETICIONES);
    if(sem_error){

        perror("Error de sem_init()");
        exit(-1);
    }

    // Variables necesarias
    int fileDes2,long_c,puerto;
    // Estructuras para el cliente y el servidor
    struct sockaddr_in server;
    struct sockaddr_in client;
    

    // Definiendo el puerto y la contraseña
    puerto = atoi(argv[1]);

    printf("El puerto definido es: %d\n",puerto);
    printf("La contraseña definida es: %s\n",password);

    // Configurando el servidor, asignandole los valores a su estructura
    server.sin_family = AF_INET; //Asigna la familia de protocolo
    server.sin_port = htons(puerto);//Prepara el puerto para ser enviado a la red
    server.sin_addr.s_addr = INADDR_ANY;//Permite que cualquier puerto se conecte
    bzero(&(server.sin_zero),8);//Rellena el arreglo con 0
    
    // Definiendo los sockets
    fileDes1 = socket(AF_INET,SOCK_STREAM,0);
    if(fileDes1 < 0){
        perror("Error en la apertura del socket");
        exit(-1);
    }

    // Asociando socket con el bind
    if(bind(fileDes1,(struct sockaddr*)&server,sizeof(struct sockaddr)) == -1){
        printf("Error en bind\n");
        exit(-1);
    }

    // Estableciendo listen a los sockets para la escucha
    if(listen(fileDes1,5) == -1){
        printf("Error en el listen\n");
        exit(-1);
    }

    printf("Servidor esperando inicio de ataques...\n");

    char passwordRcv[MAX];// Contraseña recibida
    int i = 0;

    pthread_t hilo;
    int datos[2];//contendrá el fileDescriptor y el número de ataque
    //Ciclo que se quedará recibiendo porgramas que quieran atacar
    while(!descifrada){

        // Acepta las conexiones
        fileDes2 = accept(fileDes1,(struct sockaddr*)&client,&long_c);
        datos[0] = fileDes2;
        datos[1] = (i+1);
        printf("FILE DESCRIPTOR 2: %d\n",fileDes2);
        if(fileDes2 == -1){
            printf("Error en acept()\n");
            exit(-1);
        }
        pthread_create(&hilo,NULL,recibir,datos);//Cuando se conecta un programa se ejecuta un nuevo hilo
        i++;

    }
    close(fileDes1);
    sem_destroy(&semaforo);
    return 0;   
}