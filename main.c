#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "replacementalgorithms.h"

int isDebugMode = 0;

int validateReadWrite(char rw) {
    return (rw != READ && rw != WRITE);
}

int getOffsetSize(unsigned pageSize) {
    int s = 0, tmpPageSize = pageSize;
    while (tmpPageSize > 1) {
        tmpPageSize = tmpPageSize>>1;
        s++;
    }
    return s;
}

void printResults(PageTable* pt, Statistics* stats, clock_t startTime, clock_t endTime, char file[]) {
    printf("\nTabela final:\n");
    printTable(pt, stats->algorithm, 1);
    
    printf("\nConfiguracao utilizada:\n");
    printf("Tecnica de reposicao: %s\n", stats->algorithm);
    printf("Arquivo de entrada: %s\n", file);
    printf("Tamanho das páginas: %d KB\n", stats->pageSize);
    printf("Tamanho da memoria: %d KB (%d quadros)\n\n", stats->memSize, stats->memSize/stats->pageSize);
    
    printf("Numero de acessos a memoria: %d\n", stats->accessCount);
    printf("Numero de page faults: %d\n", stats->pageFaults);
    printf("Paginas sujas escritas no disco: %d\n\n", stats->dirtyPagesWrittenDisk);

    printf("Número de páginas lidas: %d\n", stats->readPages);
    printf("Número de páginas escritas: %d\n\n", stats->writtenPages);
    
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
    if (stats->pageSize < 2) {
        printf("Invalid page size. [2, 64]\n");
        return 1;
    }

    stats->memSize = atoi(args[4]);
    /* if (stats->memSize < 128) {
        printf("Invalid memory size. [128, 16384]\n");
        return 1;
    } */

    if(args[5] != NULL) {
        if(strcmp(args[5], DEBUG) == 0) {
            *isDebugMode = 1;
        } else {
            printf("Invalid debug parameter. (debug)\n");
            return 1;
        }
    }

    return 0;
}

void accessAddresses(FILE* logFile, unsigned offsetSize, PageTable* pt, PhysMem* physMem,
                     Statistics* stats) {
    unsigned addr;
    char rw;
    while (fscanf(logFile, "%x %c", &addr, &rw) != EOF) {
        if(validateReadWrite(rw) != 0) {
            printf("Error while reading file. Parameter W/R not provided.\n");
            exit(0);
        }

        stats->accessCount++;
        unsigned pageAddress = addr >> offsetSize;

        if(isDebugMode) {
            printf(":::Memory Access:\n");
            printf(":::%x %c\n", addr, rw); 
            printf(":::Page Address: %x\n", pageAddress);
        }
        
        updatePageByAlgorithm(pt, pageAddress, physMem, stats);
        
        if(rw == READ) {
            stats->readPages++;
        } else if(rw == WRITE) {
            pt->pages[pageAddress].dirtyPage = 1;
            stats->writtenPages++;
        }

        if(isDebugMode) {
            printf(":::Page Table:\n");
            printTable(pt, stats->algorithm, 1);
        }
    }

    fclose(logFile);
}

int main(int argsCount, char* args[]) {
    FILE* logFile = NULL;
    Statistics* stats = newStatistics();

    if (validateArgs(argsCount, args, stats, &logFile, &isDebugMode) != 0) {
        printf("Error while validating arguments. Invalid Argument.\n");
        return 0;
    }

    if(isDebugMode) printf(":::DEBUG MODE:::\n");

    unsigned offsetSize = getOffsetSize(stats->pageSize*K);
    PhysMem* physMem = newPhysicalMemory(stats->memSize, stats->pageSize);
    PageTable* pt = newPageTable(offsetSize);
    
    if(isDebugMode) {
        printf("Offset Size (s): %d\n", offsetSize);
    }

    printf("Tabela inicial:\n");
    printTable(pt, stats->algorithm, 1);

    clock_t startTime = clock();
    
    accessAddresses(logFile, offsetSize, pt, physMem, stats);
    
    printResults(pt, stats, startTime, clock(), args[2]);

    free(physMem);
    free(pt);
    return 0;
}