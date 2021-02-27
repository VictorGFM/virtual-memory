#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "replacementalgorithms.h"

unsigned pageSize = 0,
    memSize = 0,
    readPages = 0,
    writtenPages = 0,
    pageFaults = 0;

char firstPrint = 1;

int bitsToShift() {
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
    printf("Tamanho da memoria: %d KB (%d páginas)\n", memSize, memSize/pageSize);
}

void printResults(pageTable* pt, clock_t startTime) {
    printf("\nTabela final:\n");
    printTable(pt);
    printf("\nTempo de execução: %fs\n", (double)(clock() - startTime) / CLOCKS_PER_SEC);
}

int validateArgs(int argsCount, char* args[], char algorithm[], FILE** logFile) {
    if (argsCount < 5) {
        printf("Invalid args count.\n");
        return 0;
    }

    strcpy(algorithm, args[1]);
    if (strcmp(algorithm, "lru") != 0 && strcmp(algorithm, "2a") != 0 && strcmp(algorithm, "fifo") != 0 && strcmp(algorithm, "custom") != 0) {
        printf("Invalid algorithm.\n");
        return 0;
    }

    *logFile = fopen(args[2], "r");
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

void readAddresses(FILE* logFile, pageTable* pt) {
    unsigned addr;
    char rw;
    int addrShiftBits = bitsToShift();
    int read;

    while ((read = fscanf(logFile, "%x %c", &addr, &rw)) != -1) {
        unsigned pageAddr = addr >> addrShiftBits;
        // printf("%d %x %c\n", read, addr, rw); //DEBUG
        memPage* page = readPage(pt, pageAddr);
        if (page == NULL) {
            pageFaults++;
            writePage(pt, pageAddr, addr);
            writtenPages++;
            if (firstPrint && isTableFull(pt)) {
                printf("\nTabela após primeira iteração completa:\n");
                printTable(pt);
                firstPrint = 0;
            }
        } else {
            readPages++;
        }
    }

}

int main(int argsCount, char* args[]) {
    FILE* logFile = NULL;
    char algorithm[7];

    if (validateArgs(argsCount, args, algorithm, &logFile) != 1) {
        return 0;
    }

    clock_t startTime = clock();
    printSettings(algorithm, args[2]);

    pageTable* pt = malloc(sizeof(pageTable) + sizeof(memPage) * (memSize/pageSize));
    pt->tableSize = memSize/pageSize, pt->pageSize = pageSize, pt->tableOccupation = 0;

    readAddresses(logFile, pt);
    
    printResults(pt, startTime);

    fclose(logFile);
    free(pt);
    return 0;
}