#ifndef REPLACEMENTALGORITHMS_H
#define REPLACEMENTALGORITHMS_H
#include <time.h>
#include <limits.h>
#include "utils.h"

struct statistics {
    unsigned pageSize;
    unsigned memSize;
    unsigned readPages;
    unsigned writtenPages;
    unsigned pageFaults;
    unsigned dirtyPagesWrittenDisk;
    unsigned accessCount;
    char algorithm[7];
}; 
typedef struct statistics statistics;

struct memPage { //TODO: confirmar formato e o que deve ser armazenado na tabela
    unsigned logicAddr;
    unsigned physicAddr;
    unsigned dirtyPage;
    unsigned timeLastAccess;
    char referenceBit;
};
typedef struct memPage memPage;

struct pageTable {
    unsigned tableSize;
    unsigned pageSize;
    unsigned lastPageIndex;
    unsigned isFirstIteration;
    memPage pages[];
};
typedef struct pageTable pageTable;

void writePageFIFO(pageTable* pt, unsigned logicAddr, unsigned physicAddr, statistics* stats);
void writePageLRU(pageTable* pt, unsigned logicAddr, unsigned physicAddr, statistics* stats);
void writePageSECONDCHANCE(pageTable* pt, unsigned logicAddr, unsigned physicAddr);
void writePageCUSTOM(pageTable* pt, unsigned logicAddr, unsigned physicAddr);

extern int isAlgorithmValid(char algorithm[]);
extern int isFirstIteration(pageTable* pt);
extern statistics* newStatistics();
extern pageTable* newPageTable(unsigned memSize, unsigned pageSize);
extern memPage* getPage(pageTable* pt, unsigned pageAddr);
extern void updatePageByAlgorithm(pageTable* pt, memPage* page, unsigned logicAddr, 
                                  unsigned addr, statistics* stats);
extern void printTable(pageTable* pt, char algorithm[]);

#endif