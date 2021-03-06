#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "replacementalgorithms.h"

int isDebugMode = 0;

int validateArgs(int argsCount, char* args[], Statistics* stats, FILE** logFile);

int getOffsetSize(unsigned pageSize);

void accessAddresses(FILE* logFile, unsigned offsetSize, PageTable* pageTable, PhysMem* physMem,
                     Statistics* stats);

int validateReadWrite(char rw);

void printResults(PhysMem* physMem, PageTable* pageTable, Statistics* stats, clock_t startTime, clock_t endTime,
                  char file[]);

int main(int argsCount, char* args[]) {
    FILE* logFile = NULL;
    Statistics* stats = newStatistics();
    srand((unsigned) time(NULL));

    if (validateArgs(argsCount, args, stats, &logFile) != 0) {
        printf("Error while validating arguments. Invalid Argument.\n");
        return 0;
    }

    if(isDebugMode) printf(":::DEBUG MODE:::\n");

    unsigned offsetSize = getOffsetSize(stats->pageSize*K);
    PhysMem* physMem = newPhysicalMemory(stats->memSize, stats->pageSize);
    PageTable* pageTable = newPageTable(offsetSize);
    
    if(isDebugMode) {
        printf("Offset Size (s): %d\n", offsetSize);
    }

    printf("\nTabela inicial:\n\n");
    printTable(physMem, pageTable, stats->algorithm);

    clock_t startTime = clock();
    
    accessAddresses(logFile, offsetSize, pageTable, physMem, stats);
    
    clock_t endTime = clock();

    printResults(physMem, pageTable, stats, startTime, endTime, args[2]);

    free(physMem);
    free(pageTable);
    return 0;
}

int validateArgs(int argsCount, char* args[], Statistics* stats, FILE** logFile) {
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
            isDebugMode = 1;
        } else {
            printf("Invalid debug parameter. (debug)\n");
            return 1;
        }
    }

    return 0;
}

int getOffsetSize(unsigned pageSize) {
    int s = 0, tmpPageSize = pageSize;
    while (tmpPageSize > 1) {
        tmpPageSize = tmpPageSize>>1;
        s++;
    }
    return s;
}

void accessAddresses(FILE* logFile, unsigned offsetSize, PageTable* pageTable, PhysMem* physMem,
                     Statistics* stats) {
    unsigned addr;
    char rw;

    while (fscanf(logFile, "%x %c", &addr, &rw) != EOF) {
        if(validateReadWrite(rw) != 0) {
            continue;
        }

        stats->accessCount++;
        unsigned pageAddress = addr >> offsetSize;

        if(isDebugMode) {
            printf(":::Memory Access:\n");
            printf(":::%x %c\n", addr, rw); 
            printf(":::Page Address: %x\n", pageAddress);
        }

        updatePageByAlgorithm(pageTable, pageAddress, physMem, stats);
        
        if(rw == READ) {
            stats->readPages++;
        } else if(rw == WRITE) {
            pageTable->pages[pageAddress].dirtyPage = 1;
            stats->writtenPages++;
        }

        if(isDebugMode) {
            printf(":::Page Table:\n");
            printTable(physMem, pageTable, stats->algorithm);
        }
    }

    fclose(logFile);
}

int validateReadWrite(char rw) {
    return (rw != READ && rw != WRITE);
}

void printResults(PhysMem* physMem, PageTable* pageTable, Statistics* stats, clock_t startTime, 
                  clock_t endTime, char file[]) {
    printf("\nTabela final:\n\n");
    printTable(physMem, pageTable, stats->algorithm);
    
    printf("\nConfigura????o utilizada:\n");
    printf("T??cnica de reposi????o: %s\n", stats->algorithm);
    printf("Arquivo de entrada: %s\n", file);
    printf("Tamanho das p??ginas: %d KB\n", stats->pageSize);
    printf("Tamanho da mem??ria: %d KB (%d quadros)\n\n", stats->memSize, stats->memSize/stats->pageSize);
    
    printf("N??mero de acessos a mem??ria: %d\n", stats->accessCount);
    printf("N??mero de page faults: %d\n", stats->pageFaults);
    printf("P??ginas sujas escritas no disco: %d\n\n", stats->dirtyPagesWrittenDisk);

    printf("N??mero de p??ginas lidas: %d\n", stats->readPages);
    printf("N??mero de p??ginas escritas: %d\n\n", stats->writtenPages);
    
    printf("Tempo de execu????o: %fs\n", (double)(endTime - startTime) / CLOCKS_PER_SEC);
}