#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


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

void start(char bb[1024], char sb[2048], struct Inodo listaInodos[5][16], char datos[10][1024], int *indiceLIL, int *indiceLBL);
void format(char bb[1024], char sb[2048], struct Inodo listaInodos[5][16], char datos[10][1024], int *indiceLIL, int *indiceLBL);
int menu();

int main(){
    int opc = 0;
    char permisosT[7] = "------";

    int LIL[15] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    int LBL[15] = {10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
    char nombre[12];
    char contenido[1008];
    int indiceLIL = 0;
    int indiceLBL = 0;
    int i, j, x, y, k;

    char bootblock[1024];
    for(i=0; i<1024; i++){
        bootblock[i] = 'b';
    }
    char superbloque[2048];
    for(i=0; i<2048; i++){
        superbloque[i] = 's';
    }
    struct Inodo listaInodos[5][16];
    char datos[10][1024];
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

    struct Directorio DirActual[64] = {0};
    DirActual[0].inodo = 2;
    strcpy(DirActual[0].nombre,".");
    DirActual[1].inodo = 2;
    strcpy(DirActual[1].nombre,"..");
    struct Directorio DirT[64];
    struct Directorio DirVali[64];

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

    start(bootblock, superbloque, listaInodos, datos, &indiceLIL, &indiceLBL);

    memcpy(DirActual, datos[0], 1024);

    //Ayuda a encontrar la ruta para pwd
    int contadorRuta=0;
    char ruta[20][12];
    strcpy(ruta[0], "root");

    //Fecha
    time_t tiempo = time(0);
    struct tm *tlocal;
    char dia[3] = "12";
    char mes[3] = "12";
    char anio[3] = "12";


    //format(bootblock, superbloque, listaInodos);

    while (opc!=5){
        for(i=0; i<=contadorRuta; i++){
                printf("%s\\", ruta[i]);
        }
        opc = menu();
        tlocal = localtime(&tiempo);
        strftime(dia,3,"%d",tlocal);
        strftime(mes,3,"%m",tlocal);
        strftime(anio,3,"%y",tlocal);

        switch (opc){
        case 1:
            printf("Nombre: ");
            gets(nombre);
            printf("Contenido: ");
            gets(contenido);
            x = LIL[indiceLIL]/16;
            y = div(LIL[indiceLIL],16).rem-1;
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
        break;
        case 2:
            
            
            printf("Nombre: ");
            gets(nombre);
            DirT[0].inodo = LIL[indiceLIL];
            DirT[1].inodo = DirActual[0].inodo;
            strcpy(DirT[0].nombre,".");
            strcpy(DirT[1].nombre,"..");

            x = LIL[indiceLIL]/16;
            y = div(LIL[indiceLIL],16).rem-1;
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
            y = div(DirActual[0].inodo,16).rem -1;
            memcpy(datos[listaInodos[x][y].tablaContenido[0]-9], DirActual, 1024);
        break;
        case 3:
            for(i=0; i<64; i++){
                if(DirActual[i].inodo){
                    printf("%d\t%s\n",DirActual[i].inodo, DirActual[i].nombre);
                }
            }
            printf("\n");
        break;
        case 4:
            for(i = 0; i<64; i++){
                if (DirActual[i].inodo){
                    x = DirActual[i].inodo/16;
                    y = div(DirActual[i].inodo,16).rem -1;
                    for(j=0; j<6; j++){
                        permisosT[j] = listaInodos[x][y].permisos[j];
                    }
                    permisosT[6] = '\0';
                    printf("%d\t%s\t%c\t%s\t%d\t%s\t%dbytes\t%d-%d-%d\n",DirActual[i].inodo, DirActual[i].nombre, listaInodos[x][y].tipos, permisosT, listaInodos[x][y].enlaces, listaInodos[x][y].usuario, listaInodos[x][y].bytes, listaInodos[x][y].dia, listaInodos[x][y].mes, listaInodos[x][y].anio);
                }
            }
            printf("\n");
        break;
        case 5:
            printf("Saliendo\n");
            format(bootblock, superbloque, listaInodos, datos, &indiceLIL, &indiceLBL);
            //instricciones para guardar datos
        break;
        case 6:
            printf("dir: >");
            gets(nombre);
            for(i=0; i<64; i++){
                if(!strcmp(DirActual[i].nombre, nombre) && DirActual[i].inodo){
                    x = DirActual[i].inodo/16;
                    y = div(DirActual[i].inodo,16).rem -1;
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
            if(i==64)
                printf("El sistema no puede encontrar la ruta especificada\n");
                
        break;
        case 7:
            printf("archivo: >");
            gets(nombre);
            for(i=0; i<64; i++){
                if(!strcmp(DirActual[i].nombre, nombre) && DirActual[i].inodo){
                    x = DirActual[i].inodo/16;
                    y = div(DirActual[i].inodo,16).rem -1;
                    if(listaInodos[x][y].tipos != 'd'){
                        printf("%s\n\n", datos[listaInodos[x][y].tablaContenido[0]-9]);
                    }else{
                        printf("no es un archivo\n");
                    }
                    break;
                }
            }
            if(i == 64){
                printf("No se encontro el archivo\n");
            }
        break;
        case 8:
            for(i=0; i<=contadorRuta; i++){
                printf("%s\\", ruta[i]);
            }
            printf("\n");
        break;
        case 9:
            printf("archivo: >");
            gets(nombre);
            for(i=0; i<64; i++){
                if(!strcmp(DirActual[i].nombre, nombre) && DirActual[i].inodo){
                    x = DirActual[i].inodo/16;
                    y = div(DirActual[i].inodo,16).rem -1;
                    if(listaInodos[x][y].tipos != 'd'){
                        listaInodos[x][y].tipos = 0;
                        indiceLBL--;
                        LBL[indiceLBL]= listaInodos[x][y].tablaContenido[0];
                        indiceLIL--;
                        LIL[indiceLIL] = DirActual[i].inodo;
                        DirActual[i].inodo = 0;
                    }else{
                        printf("no es un archivo\n");
                    }
                    break;
                }
            }
            if(i == 64){
                printf("No se encontro el archivo\n");
            }
            x = DirActual[0].inodo/16;
            y = div(DirActual[0].inodo,16).rem -1;
            memcpy(datos[listaInodos[x][y].tablaContenido[0]-9], DirActual, 1024);
        break;
        case 10:
            printf("dir: >");
            gets(nombre);
            for(i=0; i<64; i++){
                if(!strcmp(DirActual[i].nombre, nombre) && DirActual[i].inodo){
                    x = DirActual[i].inodo/16;
                    y = div(DirActual[i].inodo,16).rem -1;
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
                            printf("El directorio no está vacío\n");
                        }
                        break;
                    }
                }
            }
            if(i==64)
                printf("El sistema no puede encontrar la ruta especificada\n");
            x = DirActual[0].inodo/16;
            y = div(DirActual[0].inodo,16).rem -1;
            memcpy(datos[listaInodos[x][y].tablaContenido[0]-9], DirActual, 1024);
        break;
        default:
            break;
        }
    }
    return 0;
}
    

void start(char bb[1024], char sb[2048], struct Inodo listaInodos[5][16], char datos[10][1024], int *indiceLIL, int *indiceLBL){
    int filedoc;
    filedoc = open("Fs", O_RDONLY);
    if(filedoc == -1){
        close(filedoc);
        format(bb, sb, listaInodos, datos, indiceLIL, indiceLBL);
        return;
    }
    printf("%ld\n",read(filedoc, bb, 1024));
    printf("%ld\n",read(filedoc, sb, 1024*2));
    printf("%ld\n",read(filedoc, listaInodos, 1024 * 5));
    printf("%ld\n",read(filedoc, datos, 1024 * 10));
    read(filedoc, indiceLIL, 4);
    read(filedoc, indiceLBL, 4);    
    printf("datos leidos\n\n");
    close(filedoc);
}

void format(char bb[1024], char sb[2048], struct Inodo listaInodos[5][16], char datos[10][1024], int *indiceLIL, int *indiceLBL){
    int filedoc;
    filedoc = open("Fs", O_WRONLY|O_CREAT|O_TRUNC, 666);
    if(filedoc == -1){
        perror("Error");
        return;
    }
    printf("%ld\n",write(filedoc, bb, 1024));
    //gets();
    printf("%ld\n",write(filedoc, sb, 1024*2));
    //gets();
    printf("%ld\n",write(filedoc, listaInodos, 1024 * 5));
    //gets();
    printf("%ld\n",write(filedoc, datos, 1024 * 10));
    //gets();
    write(filedoc, indiceLIL, 4);
    write(filedoc, indiceLBL, 4);
    printf("datos escritos\n\n");
}

int menu(void){
    char opc[10];
    //printf("1.- Crear archivo\n2.- Crear directorio\n3.- Ver\n4.- Ver indos\n5.- Fin\nOpcion:");
    printf("> ");
    gets(opc);
    
    if(!strcmp(opc, "touch")){ //Crea un archivo
        return 1;
    }else if(!strcmp(opc, "mkdir")){
        return 2;
    }else if(!strcmp(opc, "ls -l")){
        return 4;
    }else if (!strcmp(opc, "ls")){
        return 3;
    }else if(!strcmp(opc, "exit")){
        return 5;
    }else if(!strcmp(opc, "cd")){
        return 6;
    }else if(!strcmp(opc, "cat")){
        return 7;
    }else if(!strcmp(opc, "pwd")){
        return 8;
    }else if(!strcmp(opc, "rm")){
        return 9;
    }else if(!strcmp(opc, "rmdir")){
        return 10;
    }else{
        printf("no se reconoce el comado\n");
        return 0;
    }
    return 0;
}