/* ----- CLIENTE QUE INTENTARÁ DESCIFRAR LA CONTRASEÑA DEL SERVIDOR-----

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
#include <netdb.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "passwords.h"

#define MAX MAX_LENGTH

int nThreads;//Número de hilos que serán ejecutados
sem_t semaforo;// Semaforo para el control de los hilos
int descifrada = 0; //Variable binaria para saber si la contraseña fue descifrada

/**
 * Funcion que ejecutará el hilo para atacar al objetivo
 * Se le enviará como parametro el fileDescriptor
 * */
void* atacar(void *fd){
    
    char *password = (char*)malloc(sizeof(char)*MAX);
    int fileDes = *(int*)fd;
    char *filePath =(char*) malloc(sizeof(char)*50);
    sprintf(filePath, "ataques/atacantes/ataque%d.lst", getpid());

    while(1){
        sem_wait(&semaforo);
        char *password = getPassword(4);
        send(fileDes,password,MAX,0);
        printf("password enviado: %s\n",password);
        writeFile(filePath,password);//Escribe la contraseña usada en su archivo
        sem_post(&semaforo);
    }
    pthread_exit(NULL);
    close(fileDes);
}


int main(int argc, char **argv){

    /**
     * Instrucciones de cómo usar el programa
     **/
    if(argc != 4){

        printf("\n\n\n\n\n");
        printf("Comando invalido.\n----- INSTRUCCIONES -----\n"
        "Debe enviar TRES parametros, estos son:\n"
        "1. La dirección IP del servidor al que se conectará.\n"
        "2. El puerto por el cual se realizará la conexión con el servidor.\n"
        "3. El número de hilos que se ejecutaran para el ataque.\nEjemplo de uso:\n");
        printf("./atacante 127.0.0.1 1025 10\n");
        printf("\n\n\n\n\n");
        exit(-1);
    }


    //Variables necesarias
    char *ip;
    int fileDes,puerto;

    ip = argv[1];//Asiga la direccion IP
    puerto = atoi(argv[2]);//Asigna el puerto por el cual se va a conectar
    nThreads = atoi(argv[3]);// Asigna el número de hilos que se van a usar en el ataque
    pthread_t hilos[nThreads];//Arreglo de los hilos
    

    // declarando Semaforo
    int sem_error = sem_init(&semaforo,0,(int)(nThreads/2));
    if(sem_error){
        perror("Error de sem_init()");
        exit(-1);
    }

    struct hostent *he;
    struct sockaddr_in server;

    // Informacion sobre la IP
    if ((he = gethostbyname(ip)) == NULL){

        printf("error en gethostbyname()\n");
        exit(-1);
    }

    // Definiendo el socket
    if ((fileDes = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("error en socket()\n");
        exit(-1);
    }

    // Configurando el servidor, asignandole los valores a su estructura
    server.sin_family = AF_INET;
    server.sin_port = htons(puerto);
    server.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(server.sin_zero),8);

    // Conexión con el servidor
    if(connect(fileDes, (struct sockaddr *)&server, sizeof(struct sockaddr))==-1){
        printf("error en connect()\n");
        exit(-1);
    }

    printf("SERVIDOR CONECTADO!\nIniciando ataques...\n");

    int i;
    //Ciclo para ejecutar los hilos
    for(i = 0; i < nThreads;i++){
        pthread_create(&hilos[i],NULL,atacar,&fileDes);
    }

    //Ciclo para esperar a que terminen los hilos
    for(int i = 0; i < nThreads;i++){
        pthread_join(hilos[i],NULL);
    }

    return 0;
}