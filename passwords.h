/**
 * Libreria que permite a los programas obtener determinadas contraseñas y escribir en un archivo determinado
 **/
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define MAX_LENGTH 40 
#define N_CHARACTERS MAX_LENGTH/5 //Maximo caracteres de la contraseña de nivel 3

char *getPasswordDic(int);
char *getPasswordRandom();
char *getPassword(int);

long n = 0;// Variable que ayuda a crear una semilla para un número al azar

/**
 * Permite obtener una contraseña al azar
 * Puede ser obtenida de un diccionario o creada totalmente al azar
 * Recibe por parametro el nivel de la contraseña
 */
char* getPassword(int level){

    n++;
    //time_t t;
    srand(time(NULL)+n);//Prepara para crear un número al azar

    int rnd = rand() % 2 + 1;//Número al azar para saber si devuelve una contraseña de diccionario o una
                            // creada totalmente al azar
    switch(level){
        case 4://En caso 4 se envia una contraseña cualquiera, de diccionario o al azar
            if(rnd == 1){ //Si es 1 retorna una contraseña del diccionario
                return getPasswordDic(0);
                break;
            }
            return getPasswordRandom();
            break;
        case 0:// En caso 0 se envía una contraseña del diccionario principal
            return getPasswordDic(0);
            break;
        case 1:// En caso 1 se envía una contraseña del diccionario de nivel 1
            return getPasswordDic(1);
            break;
        case 2:// En caso 1 se envía una contraseña del diccionario de nivel 1
            return getPasswordDic(2);
            break;
        case 3:// En caso 3 se envía una contraseña alfanumerica totalmente al azar
            return getPasswordRandom();
            break;
    }
}

/**
 * Permite obtener una contraseña alfanumerica totalmente al azar (tipo: 'aWtyQ1u7')
 */
char* getPasswordRandom(){

    /**
     * Arreglos que contendrán las letras y numero para crear contraseñas
     */
    char min[]="abcdefghijklmnopqrstuvwxyz";
	char may[]="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	char num[]="0123456789";

    n++;
    //time_t t;
    srand(time(NULL)+n);//Prepara para crear un número al azar

    int size;//Tamaño de la contraseña

    size = rand() % N_CHARACTERS + 1;//Número random que tendrá el número de caracteres de la contraseña

    char password[size];

    //Ciclo que se encarga de elegir un caracter de cualquiera de los 3 arreglos min-may-num
    for(int i = 0; i < size; i++){

		switch ( (rand() % 3+1) ) {
            case 1:
                password[i] = min[rand() % 25 + 1];
                break;
            case 2:
                password[i] = may[rand() % 25 + 1];
                break;
            case 3:
                password[i] = num[rand() % 9 + 1];
                break;
		}
    }

    password[size] = '\0';//Para terminar la cadena

    char* pass = (char*)malloc(sizeof(char)*strlen(password));
    strcpy(pass,password);
    return pass;
}

/**
 * Permite obtener una contraseña de los diccionarios
 * Recibe por parametro el nivel de la contraseña
 */
char* getPasswordDic(int level){

    FILE *file;
    char *path;
    n++;
    int choice,total;
    char *string = (char*)malloc(sizeof(char)*MAX_LENGTH);
    srand(time(NULL) + n);

    //Según el nivel elige el diccionario de donde sacará la contraseña
    if(level == 0){
        path = "diccionarios/main_dic.lst";
    }else if(level == 1){
        path = "diccionarios/pass1.lst";
    }else{
        path = "diccionarios/pass2.lst";
    }

    // Abre el archivo en modo lectura
    file = fopen(path,"r");
    if(file == NULL){
        printf("Error de archivo intentando abrir los diccionarios\n");
        exit(-1);
    }         

    int size = 0;
    //Ciclo que cuenta el número de palabras que tiene el archivo
    while(!feof(file)){
        size++;
        fgets(string,MAX_LENGTH,file);
    }
    
    choice = rand() % size + 1;//Elige un número al azar para obtener la palabra

    rewind(file);// Sitúa el cursor de lectura/escritura al principio del archivo.
    size = 0;
    //Busca la palabra elegida
    while(!feof(file)){
        size++;
        fgets(string,MAX_LENGTH,file);
        strtok(string, "\n");
        if(size == choice){
            fclose(file);
            return string;
        }
    }
}

/**
 * Permite escribir en un archivo
 * Recibe como parametro la ruta de dicho archivo y la palabra que se desea escribir
 */ 
void writeFile(char *path,char *string){

    FILE *file;
    file = fopen(path,"a");
    fputs(string,file);
    fputs("\n",file);

    fclose(file);
}

