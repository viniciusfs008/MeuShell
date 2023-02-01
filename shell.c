//BIBLIOTECAS ---------------------------------------------------------------------------------------------

#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<string.h>
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>

//VARIAVEIS GLOBAIS ---------------------------------------------------------------------------------------

#define MAX_SIZE 256
#define BUFFER_SIZE 1024
#define READP 0
#define WRITEP 1

//PROTOTIPAÇÃO ---------------------------------------------------------------------------------------------

//usada para pegar o comando e separar os tokens que o compoe
void leAnalisaComando(char**** cmds, int* n);
//printa o diretorio onde esta
void printDiretorio();
//executa os comandos
void executa(char**** cmds, int n);
//finaliza o shell
void finaliza(char* cmds, int *flag);

//MAIN -----------------------------------------------------------------------------------------------------

int main(){
    char ***cmd = NULL;
    int n, flag = 1;
    while(flag == 1){

        //le e analisa o comando
        leAnalisaComando(&cmd, &n);

        //fecha o shell 
        finaliza((**cmd), &flag);

        //executa o comando
        executa(&cmd, n);
        
    }
    return 0;
}

//IMPLEMENTAÇÕES ------------------------------------------------------------------------------------------

//usada para pegar o comando e separar os tokens que o compoe
void leAnalisaComando(char**** cmds, int* n){
    size_t size = 256;
    char *comando = (char*) malloc(size * sizeof(char));

    //printa o diretorio atual
    printDiretorio();

    //pega a linha de comando
    getline(&comando, &size, stdin);
    comando[strlen(comando) -1 ] = '\0';

    //conta o numero de barras
    int contbarra = 0;
    for (int i = 0; i < strlen(comando); i++)
        if (comando[i] == '|')
            contbarra++;
    
    //aloca um ** para cada comando
    (*cmds) = (char***) malloc((contbarra+1) * sizeof(char**));
    
    char* token = strtok(comando," ");
    
    int cont = 0, i = 0;

    while(token != NULL){
        if(token[0] == '|'){
            token = strtok(NULL," ");
            cont++;
            (*cmds)[i] = (char**) realloc((*cmds)[i], cont * sizeof(char*));
            (*cmds)[i][cont-1] = NULL;
            cont = 0;
            i++;
        }
        else{
            cont++;
            (*cmds)[i] = (char**) realloc((*cmds)[i], cont * sizeof(char*));
            (*cmds)[i][cont-1] = token;
            token = strtok(NULL," ");
        }
    }

    cont++;
    (*cmds)[i] = (char**) realloc((*cmds)[i], cont * sizeof(char*));
    (*cmds)[i][cont-1] = NULL;

    *n = contbarra;
}

//printa o diretorio onde esta
void printDiretorio(){
    char cwd[1024];
    char name[1024];
    getcwd(cwd, sizeof(cwd));
    gethostname(name, sizeof(name));
    printf("\n\033[0;32;40m@%s\033[m:\033[0;34;40m~%s\033[m >> ", name, cwd);
}

//executa os comandos
void executa(char**** cmds, int n){
    //se não for utilizar pipes
    if (n == 0){

        //cria o processo filho para rodar o comando
        pid_t pid = fork();

        //erro
        if(pid < 0){
            printf("Erro no fork");
            exit(1);
        }

        //processo filho 
        else if(pid == 0){ 
            //caso esteja trocando de diretorio
            if(((***cmds)[0] == 'c') && ((***cmds)[1] == 'd')){
                chdir ((**cmds)[1]);
            }
            //comando normal
            else if(execvp((**cmds)[0], (**cmds)) < 0){
                printf("\nComando nao reconhecido\n");
                exit(1);
            }
        }

        //processo pai - shell
        else{
            int status;
            wait(&status);
        }
    }
    //se for utilizar pipes
    else{
        int fd[2];

        //cria o pipe
        if(pipe(fd) == -1){
            printf("Erro no pipe");
            exit(1);
        }

        //cria o filho
        pid_t pid = fork();

        //erro
        if(pid < 0){
            printf("Erro no fork");
            exit(1);
        }

        // processo filho 1
        else if(pid == 0){ 
            close(fd[READP]);
            dup2(fd[WRITEP], 1);
            if(execvp((*cmds)[0][0], (*cmds)[0]) < 0){
                printf("\nComando nao reconhecido filho 1\n");
                exit(0);
            }
            close(fd[WRITEP]);
        } 

        //processo pai
        else{
            pid_t pid2 = fork();
            //erro
            if(pid2 < 0){
                printf("Erro no fork");
                exit(1);
            }

            // processo filho 2
            else if(pid2 == 0){ 
                close(fd[WRITEP]);
                dup2(fd[READP], 0);
                if(execvp((*cmds)[1][0], (*cmds)[1]) < 0){
                    printf("\nComando nao reconhecido filho 2\n");
                    exit(0);
                }
                close(fd[READP]);
            }

            //processo pai
            else{
                wait(NULL);
            }
        }
    }  
}

//finaliza o shell
void finaliza(char* cmds, int *flag){
    if((cmds[0] == 'q') && (cmds[1] == 'u') && (cmds[2] == 'i') && (cmds[3] == 't')){
        printf("\n------------------------------- Shell Do Vini Finalizado ---------------------------------\n\n\n\n");
        flag = 0;
        exit(1);
    }
}