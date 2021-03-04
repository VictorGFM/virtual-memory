#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "replacementalgorithms.h"

int isDebugMode = 0;

int validateReadLine(char rw) {
    if(rw != READ && rw != WRITE) {
        return 1;
    }

    return 0;
}

int bitsToShift(unsigned pageSize) {
    int s = 0, tmpPageSize = pageSize;
    while (tmpPageSize > 1) {
        tmpPageSize = tmpPageSize>>1;
        s++;
    }
    return s;
}

void printResults(PageTable* pt, Statistics* stats, clock_t startTime, clock_t endTime, char file[]) {
    printf("\nTabela final:\n");
    printTable(pt, stats->algorithm);
    
    printf("\nConfiguracao utilizada:\n");
    printf("Tecnica de reposicao: %s\n", stats->algorithm);
    printf("Arquivo de entrada: %s\n", file);
    printf("Tamanho das páginas: %d KB\n", stats->pageSize);
    printf("Tamanho da memoria: %d KB (%d páginas)\n\n", stats->memSize, stats->memSize/stats->pageSize);
    
    printf("Numero de acessos a memoria: %d\n", stats->accessCount);
    printf("Numero de page faults: %d\n", stats->pageFaults);
    printf("Paginas sujas escritas no disco: %d\n\n", stats->dirtyPagesWrittenDisk);
    
    printf("Tempo de execução: %fs\n", (double)(endTime - startTime) / CLOCKS_PER_SEC);
}

int validateArgs(int argsCount, char* args[], Statistics* stats, FILE** logFile, 
                 int* isDebugMode) {
    if (argsCount < 5) {
        printf("Invalid args count.\n");
        return 1;
    }

    strcpy(stats->algorithm, args[1]);
    if (!isAlgorithmValid(stats->algorithm)) {
        printf("Invalid algorithm.\n");
        return 1;
    }

    *logFile = fopen(args[2], "r");
    if (logFile == NULL) {
        printf("Invalid file.\n");
        return 1;
    }

    stats->pageSize = atoi(args[3]);
    if (stats->pageSize < 1) {
        printf("Invalid page size.\n");
        return 1;
    }

    stats->memSize = atoi(args[4]);
    if (stats->memSize < 1) {
        printf("Invalid memory size.\n");
        return 1;
    }

    if(args[5] != NULL) {
        if(strcmp(args[5], DEBUG) == 0) {
            *isDebugMode = 1;
        } else {
            printf("Invalid debug parameter.\n");
            return 1;
        }
    }

    return 0;
}

void readAddresses(FILE* logFile, PageTable* pt, Statistics* stats) {
    unsigned addr;
    char rw;
    int addrShiftBits = bitsToShift(stats->pageSize);
    int read;

    while ((read = fscanf(logFile, "%x %c", &addr, &rw)) != -1) {

        if(validateReadLine(rw) != 0) {
            printf("Error while reading file. Parameter not provided.\n");
            exit(0);
        }

        stats->accessCount++;
        unsigned pageAddr = addr >> addrShiftBits;

        if(isDebugMode) {
            printf(":::Memory Access:\n");
            printf(":::%x %c\n", addr, rw); 
            printf(":::Page Address: %x\n", pageAddr);
        }

        MemPage* page = getPage(pt, pageAddr);

        updatePageByAlgorithm(pt, page, pageAddr, addr, stats);
        
        if(rw == READ) {
            stats->readPages++;
        } else if(rw == WRITE) {
            stats->writtenPages++;
        } 

        if(isDebugMode) {
            printf(":::Page Table:\n");
            printTable(pt, stats->algorithm);
        }

        if(isFirstIteration(pt) && pt->lastPageIndex == pt->tableSize) {
            printf("\nTabela após primeira iteração completa:\n");
            printTable(pt, stats->algorithm);
        }
    }

    fclose(logFile);
}

int main(int argsCount, char* args[]) {
    FILE* logFile = NULL;
    
    Statistics* stats = newStatistics();

    //TODO: check validation
    if (validateArgs(argsCount, args, stats, &logFile, &isDebugMode) != 0) {
        printf("Error while validating arguments. Invalid Argument.\n");
        return 0;
    }

    if(isDebugMode) printf(":::DEBUG MODE:::\n");

    clock_t startTime = clock();

    PageTable* pt = newPageTable(stats->memSize, stats->pageSize);

    if(isDebugMode) {
        printf(":::Initialized Page Table:\n");
        printTable(pt, stats->algorithm);
    }

    readAddresses(logFile, pt, stats);
    
    printResults(pt, stats, startTime, clock(), args[2]);

    free(pt);
    return 0;
}