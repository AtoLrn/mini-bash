#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


#define KCYN  "\x1B[36m"
#define KGRN  "\x1B[32m"
#define RESET "\x1B[0m"

void execProg();
int waitInput ();
void trim(char * str);
void askInput ();
void flushTab (char * params[], int size);
int checkPipe(char * params[], int size);

char saisie[2048];

// Tableau de chaîne de caractères pour stocker la suite de token
char * elements[32];
char * piped[2][32];


void trim(char * str) {
    if (str[strlen(str)-1] == '\n')
        str[strlen(str)-1] = 0;
}

void askInput () {
    printf("%sCmd=> %s", KCYN, RESET);
    fflush(stdout);
}

int checkPipe(char * params[], int size) {
    int pipeFlag = -1;
    int indexOfPipe = 0;
    for (int i = 0; i < size; i++) {
        if (params[i] != NULL && params[i][0] == '|') {
            pipeFlag = i;
        }
    }
    return pipeFlag;
}

void updateElements(int index) {
    for (int i = 0; i< 32; i++) {
        if (i < index) piped[0][i] = elements[i] ;
        if (i > index) piped[1][i-(index + 1)] = elements[i] ;
    }
}

void flushTab (char * params[], int size) {
    for (int i = 0; i < size; i++) {
        params[i] = 0;
    }
}

int waitInput () {
    flushTab(elements, 32);
    fgets(saisie, 2048, stdin);
    fflush(stdin);

    trim(saisie);
    if (strlen(saisie) == 0) return 2;
    int i = 0;
    char * params = strtok( saisie, " ");

    while (params != NULL) {
        elements[i] = params;
        params = strtok ( NULL, " ");
        i++;
    }


    if (strcmp(elements[0], "exit") == 0) return 1;

    return 0;
}

void execProg() {
    int pipe_int[2], index;
    pid_t pid, pidPipe;

    pid = fork();

    index = checkPipe(elements, 32);

    if(pid < 0)
    {
        return;
    } else if (pid == 0) {
        if (index != -1) {
            pipe(pipe_int);
            updateElements(index);
            pidPipe = fork();
            if (pidPipe == 0) {
                close(STDOUT_FILENO);  //closing stdout
                dup(pipe_int[1]);         //replacing stdout with pipe write
                close(pipe_int[0]);       //closing pipe read
                close(pipe_int[1]);

                int ret = execvp(piped[0][0],piped[0]);
                perror("Error Executing cmd");
                if (ret == -1) printf("Unknown command :( \n");
                exit(1);
            } else {
                close(STDIN_FILENO);   //closing stdin
                dup(pipe_int[0]);         //replacing stdin with pipe read
                close(pipe_int[1]);       //closing pipe write
                close(pipe_int[0]);

                int ret = execvp(piped[1][0],piped[1]);
                perror("Error Executing cmd");
                if (ret == -1) printf("Unknown command :( \n");
                exit(1);
            }
        } else {
            int ret = execvp(elements[0],elements);
            perror("Error Executing cmd");
            if (ret == -1) printf("Unknown command :( \n");
        }
        exit(1);
    } else {
        if (index != -1) {
            close(pipe_int[0]);
            close(pipe_int[1]);
            wait(0);
            wait(0);
        } else {
            int status;
            wait(&status);
        }

    }
}

int main(void) {
    int input = 0;
    printf("Hello welcome to your own bash, please type 'exit' to stop \n");
    while(1) {
        askInput();
        input = waitInput();
        if(input == 1) break;
        if (input == 2) continue;
        execProg();
    }
    printf("Bye :(");
    return EXIT_SUCCESS;
}
