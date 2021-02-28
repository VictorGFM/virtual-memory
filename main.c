#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "replacementalgorithms.h"

unsigned pageSize = 0,
    memSize = 0,
    readPages = 0,
    writtenPages = 0,
    pageFaults = 0,
    dirtyPages = 0,
    accessCount = 0;

int bitsToShift() {
    int s = 0, tmpPageSize = pageSize;
    while (tmpPageSize > 1) {
        tmpPageSize = tmpPageSize>>1;
        s++;
    }
    return s;
}

void printResults(pageTable* pt, clock_t startTime, clock_t endTime, char algorithm[], char file[]) {
    printf("\nTabela final:\n");
    PrintTable(pt, algorithm);
    printf("\nTecnica de reposicao: %s\n", algorithm);
    printf("Arquivo de entrada: %s\n", file);
    printf("Tamanho das páginas: %d KB\n", pageSize);
    printf("Tamanho da memoria: %d KB (%d páginas)\n\n", memSize, memSize/pageSize);
    printf("Número de acessos: %d\n", accessCount);
    printf("Páginas lidas da memória (page faults): %d\n", pageFaults);
    printf("Páginas lidas da tabela: %d\n", readPages);
    printf("Páginas escritas: %d\n", writtenPages);
    printf("Páginas sujas: %d\n", dirtyPages);
    printf("\nTempo de execução: %fs\n", (double)(endTime - startTime) / CLOCKS_PER_SEC);
}

int validateArgs(int argsCount, char* args[], char algorithm[], FILE** logFile) {
    if (argsCount < 5) {
        printf("Invalid args count.\n");
        return 0;
    }

    strcpy(algorithm, args[1]);
    if (!IsAlgorithmValid(algorithm)) {
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

void readAddresses(FILE* logFile, pageTable* pt, char algorithm[]) {
    unsigned addr;
    char rw;
    int addrShiftBits = bitsToShift();
    int read;

    while ((read = fscanf(logFile, "%x %c", &addr, &rw)) != -1) {
        accessCount++;
        unsigned pageAddr = addr >> addrShiftBits;
        // printf("%d %x %c\n", read, addr, rw); //DEBUG
        memPage* page = GetPage(pt, pageAddr);
        if (page == NULL) {
            pageFaults++;
            if (rw == 'W') {
                writtenPages++;
                if (!IsFirstIteration(pt)) {
                    dirtyPages++;
                }
            }
            UpdatePageByAlgorithm(pt, pageAddr, addr, algorithm);
            if (IsFirstIteration(pt) && IsTableFull(pt)) {
                printf("\nTabela após primeira iteração completa:\n");
                PrintTable(pt, algorithm);
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

    pageTable* pt = NewPageTable(memSize, pageSize);

    readAddresses(logFile, pt, algorithm);
    
    printResults(pt, startTime, clock(), algorithm, args[2]);

    fclose(logFile);
    free(pt);
    return 0;
}