#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>
#include <string.h>

#define SERVER_ADDRESS  "127.0.0.1"     /* server IP */
#define PORT            8080 

struct instruction {
    int code;
    char nombre[12];
    char contenido[1008];
};

int main(){
    char opc[20];
    int op = -1;
    struct instruction ins;
    int i;
    //Variables para comunicación
    char buff_r[2000];
    char ban;
    int sockfd; 
    struct sockaddr_in servaddr; 
    
    /* Socket creation */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) 
    { 
        printf("CLIENT: socket creation failed...\n"); 
        return -1;  
    } 
    else
    {
        printf("CLIENT: Socket successfully created..\n"); 
    }
    
    
    memset(&servaddr, 0, sizeof(servaddr));

    /* assign IP, PORT */
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr( SERVER_ADDRESS ); 
    servaddr.sin_port = htons(PORT); 
  
    /* try to connect the client socket to server socket */
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) 
    { 
        printf("connection with the server failed...\n");  
        return -1;
    } 
    
    printf("connected to the server..\n"); 





    ins.code = -1;
    while(ins.code != 5){
        strcpy(ins.nombre, "");
        strcpy(ins.contenido, "");
        if (ins.code != 0){
            memset(buff_r, 0, sizeof(buff_r));
            //read(sockfd, buff_r, sizeof(buff_r));
            recv(sockfd, buff_r,sizeof(buff_r),0 );
        }
        // i=0;
        // while(buff_r[i] != '+'){
        //     printf("%c", buff_r[i]);
        //     i++;
        // }
        printf("%s", buff_r);
        printf("> ");
        gets(opc);
        if(!strcmp(opc, "touch")){          // Crea un archivo
            ins.code = 1;
            printf("nombre>");
            gets(ins.nombre);
            printf("contenido>");
            gets(ins.contenido);
        }else if(!strcmp(opc, "mkdir")){    // Crea un directorio
            ins.code = 2;
            printf("nombre>");
            gets(ins.nombre);
        }else if(!strcmp(opc, "ls")){       // Muestra el contenido del directorio actual
            ins.code = 3;
        }else if (!strcmp(opc, "ls -l")){   // Muestra el contenido del directorio actual con más detalle
            ins.code = 4;
        }else if(!strcmp(opc, "exit")){     // Cierra el simulador
            ins.code = 5;
        }else if(!strcmp(opc, "cd")){       // Cambia de directorio
            ins.code = 6;
            printf("nombre>");
            gets(ins.nombre);
        }else if(!strcmp(opc, "cat")){      // Muestra el contenido de cd un archivo
            ins.code = 7;
            printf("nombre>");
            gets(ins.nombre);
        }else if(!strcmp(opc, "pwd")){      // Muestra la ruta completa del directorio actual
            ins.code = 8;
        }else if(!strcmp(opc, "rm")){       // Elimina un archivo
            ins.code = 9;
            printf("nombre>");
            gets(ins.nombre);
        }else if(!strcmp(opc, "rmdir")){    // Elimina un directorio vacío
            ins.code = 10;
            printf("nombre>");
            gets(ins.nombre);
        }else if(!strcmp(opc, "round-robin")){
            ins.code = 11;
        }else{
            printf("no se reconoce el comado\n");
            printf("ayuda:\n");
            printf("--touch\t\tcrear archivo\n");
            printf("--mkdir\t\tcrear un directorio\n");
            printf("--ls\t\tmuestra el contenido del directorio actual\n");
            printf("--ls -l\t\tmuestra el contenido con detalle\n");
            printf("--exit\t\tsalir\n");
            printf("--cd\t\tcambiar de directorio\n");
            printf("--cat\t\tmuestra el contenido de un archivo\n");
            printf("--pwd\t\tmuestra la ruta del directorio actual\n");
            printf("--rm\t\teliminar un archivo\n");
            printf("--rmdir\t\teliminar un directorio vacío\n\n");
            printf("--round-robin\t\tejecuta el algoritmo en el servidor\n\n");
            ins.code = 0;
        }
        // Enviar instruccion al servidor
        if (ins.code != 0){
            //write(sockfd, &ins, sizeof(ins));
            
            if( send(sockfd, &ins, sizeof(ins), 0) < 0 ){
                printf("error");
            }
        }
        if (ins.code == 3 || ins.code == 4 || ins.code == 6 || ins.code == 7 || ins.code == 8 || ins.code == 9 || ins.code == 10){
            //strcpy(buff_r, "");
            memset(buff_r, 0, sizeof(buff_r));
            recv(sockfd, buff_r, sizeof(buff_r),0);
            // i=0;
            // while(buff_r[i] != '+'){
            //     printf("%c", buff_r[i]);
            //     i++;
            // }
            printf("%s", buff_r);
        }
    }
    /* close the socket */
    close(sockfd); 
}
