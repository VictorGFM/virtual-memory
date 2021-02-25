#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "replacementalgorithms.h"

int pageSize = 0, memSize = 0;

int bitsToIgnore() {
    int s = 0, tmpPageSize = pageSize;
    while (tmpPageSize > 1) {
        tmpPageSize = tmpPageSize>>1;
        s++;
    }
    return s;
}

void printSettings(char algorithm[], char file[]) {
    printf("Executando o simulador...\n");
    printf("Tecnica de reposicao: %s\n", algorithm);
    printf("Arquivo de entrada: %s\n", file);
    printf("Tamanho das páginas: %d KB\n", pageSize);
    printf("Tamanho da memoria: %d KB\n", memSize);
}

int validateArgs(int argsCount, char* args[], char algorithm[], FILE* logFile) {
    if (argsCount < 5) {
        printf("Invalid args count.\n");
        return 0;
    }

    strcpy(algorithm, args[1]);
    if (strcmp(algorithm, "lru") != 0 && strcmp(algorithm, "2a") != 0 && strcmp(algorithm, "fifo") != 0 && strcmp(algorithm, "custom") != 0) {
        printf("Invalid algorithm.\n");
        return 0;
    }

    logFile = fopen(args[2], "r");
    if (logFile == NULL) {
        printf("Invalid file.\n");
        return 0;
    }

    pageSize = atoi(args[3]);
    if (pageSize < 1) {
        printf("Invalid page size.\n");
        return 0;
    }

    memSize = atoi(args[4]);
    if (memSize < 1) {
        printf("Invalid memory size.\n");
        return 0;
    }

    return 1;
}

void readAddresses(FILE* logFile) {
    
}

int main(int argsCount, char* args[]) {
    FILE* logFile = NULL;
    char algorithm[7];
    if (validateArgs(argsCount, args, algorithm, logFile) != 1) {
        return 0;
    }

    printSettings(algorithm, args[2]);

    readAddresses(logFile);

    return 0;
}