#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

/* sockets */
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
/* server parameters */
#define SERV_PORT       8080              /* port */
//#define SERV_HOST_ADDR "192.168.0.21"     /* IP, only IPV4 support  */
#define SERV_HOST_ADDR "127.0.0.1"
#define BUF_SIZE        100               /* Buffer rx, tx max size  */
#define BACKLOG         5                 /* Max. client pending connections  */

#define limite 30

struct Inodo{
    char tipos;
    char permisos[6];
    char enlaces;
    char usuario[12];
    int bytes;
    char dia;
    char mes;
    char anio;
    //Para llenar los 64 bytes;
    int tablaContenido[9];
    char dummy;
};

struct Directorio{
    int inodo;
    char nombre[12];
};

struct instruction {
    int code;
    char nombre[12];
    char contenido[1008];
};

//Declaracion de variables globales
int LIL[15] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
int LBL[15] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
int indiceLIL = 0;
int indiceLBL = 0;
struct Inodo listaInodos[5][16];
char dia[3] = "12";
char mes[3] = "12";
char anio[3] = "12";
char datos[10][1024];
struct Directorio DirActual[64] = {0};
char bootblock[1024];
char superbloque[2048];
int contadorRuta=0;
char ruta[20][12];

void start();
void format();
int menu();

int touch(char nombre[12], char contenido[1008]);
int mkdir(char nombre[12]);
int ls(char *);
int ls_l(char *);
int my_exit();
int cd(char nombre[12], char*);
int cat(char nombre[12], char*);
int pwd(char *);
int my_rm(char nombre[12], char*);
int my_rmdir(char nombre[12], char*);

void RR(char *proceso, char *tiempo, unsigned char numero);

int main(){
    int opc = 0;    
    int i, j, x, y, k;
    char stringhelper[256];
    char codigosingresados[limite];
    char tiempoprocesos[limite];
    char contprocesos = 0;
    //sockets
    int sockfd, connfd ;  /* listening socket and connection socket file descriptors */
    unsigned int len;     /* length of client address */
    struct sockaddr_in servaddr, client;

    int  len_r, len_tx = 0;                     /* received and sent length, in bytes */
    char buff_s[2000];
    struct instruction buff_r;   /* buffers for reception  */

    //Inicializacion del sistema de archivos
    for(i=0; i<1024; i++){
        bootblock[i] = 'b';
    }
    for(i=0; i<2048; i++){
        superbloque[i] = 's';
    }
    for(i=0; i<10; i++){
        for(j=0; j<1024; j++){
            datos[i][j] = 'd';
        }
    }
    for(i=0; i<5; i++){
        for(j=0; j<16; j++){
            listaInodos[i][j].tipos = 0;
            strcpy(listaInodos[i][j].permisos, "rwxrwx");
            listaInodos[i][j].enlaces = 1;
            strcpy(listaInodos[i][j].usuario, "root");
            listaInodos[i][j].bytes =1024;
            listaInodos[i][j].dia = 'd';
            listaInodos[i][j].mes = 'm';
            listaInodos[i][j].anio = 'a';
            for(k=0; k<9; k++){
                listaInodos[i][j].tablaContenido[k] = 0;
            }
        }
    }
    DirActual[0].inodo = 2;
    strcpy(DirActual[0].nombre,".");
    DirActual[1].inodo = 2;
    strcpy(DirActual[1].nombre,"..");
    listaInodos[0][1].tipos = 'd';
    strcpy(listaInodos[0][1].permisos, "rwxrwx");
    listaInodos[0][1].enlaces = 1;
    strcpy(listaInodos[0][1].usuario, "root");
    listaInodos[0][1].bytes =1024;
    listaInodos[0][1].dia = 21;
    listaInodos[0][1].mes = 05;
    listaInodos[0][1].anio = 21;
    listaInodos[0][1].tablaContenido[0] = 9;
    memcpy(datos[0], DirActual, 1024);
    start();
    memcpy(DirActual, datos[0], 1024);
    strcpy(ruta[0], "root");
    //Fecha
    time_t tiempo = time(0);
    struct tm *tlocal;
    //format(bootblock, superbloque, listaInodos);


    /* socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if( sockfd == -1){
        printf("Error");
        return -1;
    }else{
        printf("Socket creado");
    }
    /* clear structure */
    memset(&servaddr, 0, sizeof(servaddr));
    /* assign IP, SERV_PORT, IPV4 */
    servaddr.sin_family      = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR); 
    servaddr.sin_port        = htons(SERV_PORT); 

    /* Bind socket */
    if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) 
    { 
        fprintf(stderr, "[SERVER-error]: socket bind failed. %d: %s \n", errno, strerror( errno ));
        return -1;
    } 
    else
    {
        printf("[SERVER]: Socket successfully binded \n");
    }

    /* Listen */
    if ((listen(sockfd, BACKLOG)) != 0) 
    { 
        fprintf(stderr, "[SERVER-error]: socket listen failed. %d: %s \n", errno, strerror( errno ));
        return -1;
    } 
    else
    {
        printf("[SERVER]: Listening on SERV_PORT %d \n\n", ntohs(servaddr.sin_port) ); 
    }
    len = sizeof(client); 

    /* Accept the data from incoming sockets in a iterative way */
      while(1)
      {
        connfd = accept(sockfd, (struct sockaddr *)&client, &len); 
        if (connfd < 0) 
        { 
            fprintf(stderr, "[SERVER-error]: connection not accepted. %d: %s \n", errno, strerror( errno ));
            return -1;
        } 
        else
        {              
            while(1) /* read data from a client socket till it is closed */ 
            {  
                //strcpy(buff_s, "");
                memset(buff_s, 0, sizeof(buff_s));
                for(i=0; i<=contadorRuta; i++){
                    sprintf(stringhelper, "%s\\", ruta[i]);
                    strcat(buff_s, stringhelper);
                }
                printf("%s\n", buff_s);
                //write(connfd, buff_s, strlen(buff_s));
                send(connfd, buff_s, sizeof(buff_s),0);
                memset(buff_s, 0, sizeof(buff_s));
                /* read client message, copy it into buffer */
                len_r = read(connfd, &buff_r, sizeof(buff_r));  
                
                if(len_r == -1)
                {
                    fprintf(stderr, "[SERVER-error]: connfd cannot be read. %d: %s \n", errno, strerror( errno ));
                }
                else if(len_r == 0) /* if length is 0 client socket closed, then exit */
                {
                    printf("[SERVER]: client socket closed \n\n");
                    close(connfd);
                    break; 
                }
                else
                {
                    printf("[SERVER] instruction code: %d \n", buff_r.code);
                    printf("[SERVER] data.name: %s \n", buff_r.nombre);
                    printf("[SERVER] data.contenido: %s \n", buff_r.contenido);
                    
                    tlocal = localtime(&tiempo);
                    strftime(dia,3,"%d",tlocal);
                    strftime(mes,3,"%m",tlocal);
                    strftime(anio,3,"%y",tlocal);
                    memset(buff_s, 0, sizeof(buff_s));
                    // Almacenar los códgos ingresados para hacer una simulacion de cómo se resolveria por round robin
                    if(buff_r.code != 11){
                        if(contprocesos < limite) {
                            codigosingresados[contprocesos] = buff_r.code;
                            contprocesos++;
                        }else{
                            printf("[SERVER] limite para almacenar procesos alcansado, los próximos procesos no formaran parate del algoritmo round-robin\nejecute el proceso round-robin para poder agregar más\n");
                        }
                    }
                    switch (buff_r.code){
                        case 1: touch(buff_r.nombre, buff_r.contenido);
                        break;
                        case 2: mkdir(buff_r.nombre);
                        break;
                        case 3: ls(buff_s);
                                write(connfd, buff_s, strlen(buff_s));
                        break;
                        case 4: ls_l(buff_s);
                                write(connfd, buff_s, strlen(buff_s));
                        break;
                        case 5: my_exit();
                        break;
                        case 6: cd(buff_r.nombre, buff_s);
                                write(connfd, buff_s, strlen(buff_s));
                        break;
                        case 7: cat(buff_r.nombre, buff_s);
                                write(connfd, buff_s, strlen(buff_s));
                        break;
                        case 8: pwd(buff_s);
                                write(connfd, buff_s, strlen(buff_s));
                        break;
                        case 9: my_rm(buff_r.nombre, buff_s);
                                write(connfd, buff_s, strlen(buff_s));
                        break;
                        case 10: my_rmdir(buff_r.nombre, buff_s);
                                write(connfd, buff_s, strlen(buff_s));
                        break;
                        case 11: RR(codigosingresados, tiempoprocesos, contprocesos);
                                 contprocesos = 0;
                        break;
                        default:
                        break;
                    }
                }            
            }  
        }                      
    }
    return 0;
}
    

void start(){
    int filedoc;
    filedoc = open("Fs", O_RDONLY);
    if(filedoc == -1){
        close(filedoc);
        format();
        return;
    }
    printf("%ld\n",read(filedoc, bootblock, 1024));
    printf("%ld\n",read(filedoc, superbloque, 1024*2));
    printf("%ld\n",read(filedoc, listaInodos, 1024 * 5));
    printf("%ld\n",read(filedoc, datos, 1024 * 10));
    read(filedoc, &indiceLIL, 4);
    read(filedoc, &indiceLBL, 4);    
    printf("datos leidos\n\n");
    close(filedoc);
}

void format(){
    int filedoc;
    filedoc = open("Fs", O_WRONLY|O_CREAT|O_TRUNC, 666);
    if(filedoc == -1){
        perror("Error");
        return;
    }
    printf("%ld\n",write(filedoc, bootblock, 1024));
    //gets();
    printf("%ld\n",write(filedoc, superbloque, 1024*2));
    //gets();
    printf("%ld\n",write(filedoc, listaInodos, 1024 * 5));
    //gets();
    printf("%ld\n",write(filedoc, datos, 1024 * 10));
    //gets();
    write(filedoc, &indiceLIL, 4);
    write(filedoc, &indiceLBL, 4);
    printf("datos escritos\n\n");
}


int touch(char nombre[12], char contenido[1008]){
    int x, y, i;
    x = LIL[indiceLIL]/16;
    //y = div(LIL[indiceLIL],16).rem-1;
    y = (LIL[indiceLIL]%16) -1;
    listaInodos[x][y].tipos = '-';
    strcpy(listaInodos[x][y].permisos, "rw-r--");
    listaInodos[x][y].enlaces = 1;
    strcpy(listaInodos[x][y].usuario, "root");
    listaInodos[x][y].bytes = strlen(contenido);
    listaInodos[x][y].dia = atoi(dia);
    listaInodos[x][y].mes = atoi(mes);
    listaInodos[x][y].anio = atoi(anio);
    listaInodos[x][y].tablaContenido[0] = LBL[indiceLBL];
    
    memcpy(datos[LBL[indiceLBL]-9], contenido, strlen(contenido)+1);
    indiceLBL++;
    for(i=0; i<64; i++){
        if(!DirActual[i].inodo){
            DirActual[i].inodo = LIL[indiceLIL];
            indiceLIL++;
            strcpy(DirActual[i].nombre, nombre);
            break;
        }
    }
    x = DirActual[0].inodo/16;
    y = div(DirActual[0].inodo,16).rem -1;
    memcpy(datos[listaInodos[x][y].tablaContenido[0]-9], DirActual, 1024);
    printf("fin touch\n");
    return 1;
}

int mkdir(char nombre[12]){
    int x, y, i;
    static struct Directorio DirT[64];
    DirT[0].inodo = LIL[indiceLIL];
    DirT[1].inodo = DirActual[0].inodo;
    strcpy(DirT[0].nombre,".");
    strcpy(DirT[1].nombre,"..");

    x = LIL[indiceLIL]/16;
    y = (LIL[indiceLIL]%16) -1;
    listaInodos[x][y].tipos = 'd';
    strcpy(listaInodos[x][y].permisos, "rwxrwx");
    listaInodos[x][y].enlaces = 1;
    strcpy(listaInodos[x][y].usuario, "root");
    listaInodos[x][y].bytes = 1024;
    listaInodos[x][y].dia = atoi(dia);
    listaInodos[x][y].mes = atoi(mes);
    listaInodos[x][y].anio = atoi(anio);
    listaInodos[x][y].tablaContenido[0] = LBL[indiceLBL];
    
    memcpy(datos[LBL[indiceLBL]-9], DirT, 1024);
    indiceLBL++;
    for(i=0; i<64; i++){
        if(!DirActual[i].inodo){
            DirActual[i].inodo = LIL[indiceLIL];
            indiceLIL++;
            strcpy(DirActual[i].nombre, nombre);
            break;
        }
    }
    x = DirActual[0].inodo/16;
    y = (DirActual[0].inodo%16) -1;
    memcpy(datos[listaInodos[x][y].tablaContenido[0]-9], DirActual, 1024);
    return 1;
}
int ls(char *ret){
    int i;
    char stringhelper[256];
    for(i=0; i<64; i++){
        if(DirActual[i].inodo){
            sprintf(stringhelper, "%d\t%s\n",DirActual[i].inodo, DirActual[i].nombre);
            strcat(ret, stringhelper);
        }
    }
    printf("fin ls\n");
    return 1;
}
int ls_l(char *ret){
    int i, j, x, y;
    char permisosT[7] = "------";
    char stringhelper[256];
    for(i = 0; i<64; i++){
        if (DirActual[i].inodo){
            x = DirActual[i].inodo/16;
            //y = div(DirActual[i].inodo,16).rem -1;
            y = (DirActual[i].inodo%16) -1;
            for(j=0; j<6; j++){
                permisosT[j] = listaInodos[x][y].permisos[j];
            }
            permisosT[6] = '\0';
            sprintf(stringhelper,"%d\t%s\t%c\t%s\t%d\t%s\t%dbytes\t%d-%d-%d\n",DirActual[i].inodo, DirActual[i].nombre, listaInodos[x][y].tipos, permisosT, listaInodos[x][y].enlaces, listaInodos[x][y].usuario, listaInodos[x][y].bytes, listaInodos[x][y].dia, listaInodos[x][y].mes, listaInodos[x][y].anio);
            strcat(ret, stringhelper);
        }
    }
    printf("fin ls -l\n");
    return 1;
}
int my_exit(){
    printf("Saliendo\n");
    format();
    //instricciones para guardar datos
    return 1;
}
int cd(char nombre[12], char *ret){
    int i, x, y;
    for(i=0; i<64; i++){
        if(!strcmp(DirActual[i].nombre, nombre) && DirActual[i].inodo){
            x = DirActual[i].inodo/16;
            y = (DirActual[i].inodo%16) -1;
            if(listaInodos[x][y].tipos == 'd'){
                memcpy(DirActual, datos[listaInodos[x][y].tablaContenido[0]-9], 1024);
                if(!strcmp(nombre, "..")){
                    if(strcmp(ruta[contadorRuta], "root"))
                        contadorRuta--;
                }else{
                    if(strcmp(nombre, ".")){
                        contadorRuta++;
                        strcpy(ruta[contadorRuta], nombre);
                    }
                    
                }
                break;
            }
        }
    }
    if(i==64){
        sprintf(ret, "El sistema no puede encontrar la ruta especificada %s.\n", nombre);
        return 0;
    }
    sprintf(ret, "Dir: %s\n", nombre);
    return 1;
}
int cat(char nombre[12], char *ret){
    int i, x, y;
    for(i=0; i<64; i++){
        if(!strcmp(DirActual[i].nombre, nombre) && DirActual[i].inodo){
            x = DirActual[i].inodo/16;
            y = (DirActual[i].inodo%16) -1;
            if(listaInodos[x][y].tipos != 'd'){
                sprintf(ret,"%s\n\n", datos[listaInodos[x][y].tablaContenido[0]-9]);
            }else{
                sprintf(ret,"no es un archivo\n");
            }
            break;
        }
    }
    if(i == 64){
        sprintf(ret,"No se encontro el archivo\n");
    }
    printf("%s", ret);
    return 1;
}
int pwd(char * ret){
    int i;
    char stringhelper[256];
    for(i=0; i<=contadorRuta; i++){
        sprintf(stringhelper, "%s\\", ruta[i]);
        strcat(ret, stringhelper);
    }
    printf("\n");
    return 1;
}
int my_rm(char nombre[12], char *ret){
    int i, x, y;
    for(i=0; i<64; i++){
        if(!strcmp(DirActual[i].nombre, nombre) && DirActual[i].inodo){
            x = DirActual[i].inodo/16;
            y = (DirActual[i].inodo%16) -1;
            if(listaInodos[x][y].tipos != 'd'){
                listaInodos[x][y].tipos = 0;
                indiceLBL--;
                LBL[indiceLBL]= listaInodos[x][y].tablaContenido[0];
                indiceLIL--;
                LIL[indiceLIL] = DirActual[i].inodo;
                DirActual[i].inodo = 0;
            }else{
                sprintf(ret, "no es un archivo\n");
                return 0;
            }
            break;
        }
    }
    if(i == 64){
        sprintf(ret,"No se encontro el archivo\n");
        return 0;
    }
    x = DirActual[0].inodo/16;
    y = (DirActual[0].inodo%16) -1;
    sprintf(ret, "Se eliminó: %s\n", nombre);
    memcpy(datos[listaInodos[x][y].tablaContenido[0]-9], DirActual, 1024);
    return 1;
}
int my_rmdir(char nombre[12], char *ret){
    int i, x, y, j;
    struct Directorio DirVali[64];
    for(i=0; i<64; i++){
        if(!strcmp(DirActual[i].nombre, nombre) && DirActual[i].inodo){
            x = DirActual[i].inodo/16;
            y = (DirActual[i].inodo%16) -1;
            if(listaInodos[x][y].tipos == 'd'){
                memcpy(DirVali, datos[listaInodos[x][y].tablaContenido[0]-9], 1024);
                for(j=2; j<64; j++){
                    if(DirVali[j].inodo){
                        break;
                    }
                }
                if(j==64){
                    listaInodos[x][y].tipos = 0;
                    indiceLBL--;
                    LBL[indiceLBL]= listaInodos[x][y].tablaContenido[0];
                    indiceLIL--;
                    LIL[indiceLIL] = DirActual[i].inodo;
                    DirActual[i].inodo = 0;
                }else{
                   sprintf(ret,"El directorio no está vacío\n");
                    return 0;
                }
                break;
            }
        }
    }
    if(i==64){
        sprintf(ret, "El sistema no puede encontrar la ruta especificada\n");
        return 0;
    }
    x = DirActual[0].inodo/16;
    y = (DirActual[0].inodo%16) -1;
    sprintf(ret, "Se eliminó: %s\n", nombre);
    memcpy(datos[listaInodos[x][y].tablaContenido[0]-9], DirActual, 1024);
    return 1;
}

//-------------Round robin-------------------
void RR(char *proceso, char *tiempo, unsigned char numero){
    char quantum = 2;
    char numprocesosact = numero;
    char i = 0;
    char tiempototal = 0;
    char tiempoaux[5];
    char pro[limite][20];

    for(i=0; i< numero; i++){
        switch(proceso[i]){
            case 1:
                strcpy(pro[i], "touch");
                tiempo[i] = 2;
            break;
            case 2:
                strcpy(pro[i], "mkdir");
                tiempo[i] = 2;
            break;
            case 3:
                strcpy(pro[i], "ls");
                tiempo[i] = 2;
            break;
            case 4:
                strcpy(pro[i], "ls -l");
                tiempo[i] = 2;
            break;
            case 5:
                strcpy(pro[i], "exit");
                tiempo[i] = 2;
            break;
            case 6:
                strcpy(pro[i], "cd");
                tiempo[i] = 2;
            break;
            case 7:
                strcpy(pro[i], "cat");
                tiempo[i] = 2;
            break;
            case 8:
                strcpy(pro[i], "pwd");
                tiempo[i] = 2;
            break;
            case 9:
                strcpy(pro[i], "rm");
                tiempo[i] = 2;
            break;
            case 10:
                strcpy(pro[i], "rmdir");
                tiempo[i] = 2;
            break;
        }
    }

    memcpy(tiempoaux, tiempo, numero);
    i=0;
    while(numprocesosact){
        if(tiempoaux[i] > 0){
            tiempoaux[i] -= quantum;
            tiempototal += quantum;
            if(tiempoaux[i] < 0){
                tiempototal += tiempoaux[i];
            }
            if(tiempoaux[i] <= 0){
                tiempoaux[i] = 0;
                numprocesosact -= 1;
                printf("tiempo final del proceso %s es: %d\n", pro[i], tiempototal);
                printf("servicio del proceso %s es: %d\n", pro[i], tiempototal-i);
                printf("tiempo de espera de %s es: %d\n", pro[i], (tiempototal-i)- tiempo[i]);
                printf("Indice del servicio del proceso %s es: %f\n", pro[i], (float)tiempo[i]/(tiempototal-i));
                printf("\n");
            }
        }
        i++;
        if(i == numero){
            i = 0;
        }
    }
}