#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


char saisie[2048];

// Tableau de chaîne de caractères pour stocker la suite de token
char * elements[32];


void trim(char * str) {
    if (str[strlen(str)-1] == '\n')
        str[strlen(str)-1] = 0;
}

void askInput () {
    printf("Cmd=> ");
    fflush(stdout);
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
    int pipe[2];
    pid_t pid;

    pid = fork();

    if(pid < 0)
    {
        return;
    } else if (pid == 0) {
        int ret = execvp(elements[0],elements);
        if (ret == -1) printf("Unknown command :( \n");
        //printf("enfant");
        exit(1);
    } else {
        int status;
        wait(&status);
        //printf("parents");
    }
}

int main(void) {
    printf("Hello welcome to your own bash, please type 'exit' to stop \n");
    while(1) {
        askInput();
        if(waitInput()) break;
        execProg();
    }
    printf("Bye :(");
    return EXIT_SUCCESS;
}
