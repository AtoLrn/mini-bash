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
void piper(int pipeVar[2], char * command[10][20], int size, int it);
void cleanCommand();

char saisie[2048];

// Tableau de chaîne de caractères pour stocker la suite de token
char * elements[64];
char * command[5][20];

void piper(int pipeVar[2], char * command[10][20], int size, int it) {
    int inner_pipe[2];
    if (it == size-1) {

        close(STDIN_FILENO);   //closing stdin
        dup(pipeVar[0]);         //replacing stdin with pipe read
        close(pipeVar[1]);       //closing pipe write
        close(pipeVar[0]);

        execvp(command[it][0], command[it]);
        exit(1);
    } else if (it == 0) {
        pipe(inner_pipe);
        pid_t pid = fork();

        if (pid == 0) {
            close(STDOUT_FILENO);  //closing stdout
            dup(inner_pipe[1]);         //replacing stdout with pipe write
            close(inner_pipe[0]);       //closing pipe read
            close(inner_pipe[1]);

            execvp(command[it][0], command[it]);
            exit(1);
        } else {
            close(pipeVar[0]);
            close(pipeVar[1]);
            it++;
            piper (inner_pipe, command, size, it);
            exit(1);
        }

    } else {
        pid_t pid;
        pipe(inner_pipe);
        pid = fork();
        if (pid == 0) {

            close(STDOUT_FILENO);  //closing stdout
            close(STDIN_FILENO);  //closing stdin
            dup(pipeVar[0]);
            close(pipeVar[1]);       //closing write read
            close(pipeVar[0]);

            dup(inner_pipe[1]);         //replacing stdout with pipe write
            close(inner_pipe[0]);       //closing pipe read
            close(inner_pipe[1]);

            execvp(command[it][0], command[it]);
            exit(1);
        } else {
            close(pipeVar[0]);
            close(pipeVar[1]);
            it++;
            piper(inner_pipe, command, size, it);
            exit(1);
        }
    }

}

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

void flushTab (char * params[], int size) {
    for (int i = 0; i < size; i++) {
        params[i] = 0;
    }
}

int waitInput () {
    flushTab(elements, 64);
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

int createCommand() {
    int cmd_idx = 0;
    int pipe_index = 0;
    for (int i = 0; i < 64;i++) {
        if (elements[i] == NULL) break;
        if (elements[i][0] == '|') {
            cmd_idx++;
            pipe_index = i;
            continue;
        }
        command[cmd_idx][i-(pipe_index + cmd_idx)] = elements[i];
    }

    return cmd_idx;
}

void cleanCommand() {
    for (int i = 0; i < 5;i++) {
        for (int y = 0; y< 20;y++) {
            command[i][y] = 0;
        }
    }
}

void execProg() {
    int pipe_int[2], index;
    pid_t pid, pidPipe;

    int size = createCommand();

    if (size != 0) {
        pipe(pipe_int);
        pid = fork();

        if (pid == 0) {
            piper(pipe_int, command, size+1, 0);
            exit(1);
        } else {
            int status;
            wait(&status);
        }

    } else {
        pid = fork();

        if (pid == 0) {
            int ret = execvp(elements[0],elements);
            perror("Error Executing cmd");
            if (ret == -1) printf("Unknown command :( \n");
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
        cleanCommand();
        execProg();
    }
    printf("Bye :( \n");
    return EXIT_SUCCESS;
}
