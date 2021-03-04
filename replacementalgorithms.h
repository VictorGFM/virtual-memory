#ifndef REPLACEMENTALGORITHMS_H
#define REPLACEMENTALGORITHMS_H
#include <time.h>
#include <limits.h>
#include "constants.h"

struct Statistics {
    unsigned pageSize;
    unsigned memSize;
    unsigned readPages;
    unsigned writtenPages;
    unsigned pageFaults;
    unsigned dirtyPagesWrittenDisk;
    unsigned accessCount;
    char algorithm[7];
}; 
typedef struct Statistics Statistics;

struct MemPage { //TODO: confirmar formato e o que deve ser armazenado na tabela
    unsigned logicAddr;
    unsigned physicAddr;
    unsigned dirtyPage;
    unsigned timeLastAccess;
    char referenceBit;
};
typedef struct MemPage MemPage;

struct PageTable {
    unsigned tableSize;
    unsigned pageSize;
    unsigned lastPageIndex;
    unsigned isFirstIteration;
    MemPage pages[];
};
typedef struct PageTable PageTable;

void writePageFIFO(PageTable* pt, unsigned logicAddr, unsigned physicAddr, Statistics* stats);
void writePageLRU(PageTable* pt, unsigned logicAddr, unsigned physicAddr, Statistics* stats);
void writePageSECONDCHANCE(PageTable* pt, unsigned logicAddr, unsigned physicAddr, Statistics* stats);
void writePageCUSTOM(PageTable* pt, unsigned logicAddr, unsigned physicAddr);

extern int isAlgorithmValid(char algorithm[]);
extern int isFirstIteration(PageTable* pt);
extern Statistics* newStatistics();
extern PageTable* newPageTable(unsigned memSize, unsigned pageSize);
extern MemPage* getPage(PageTable* pt, unsigned pageAddr);
extern void updatePageByAlgorithm(PageTable* pt, MemPage* page, unsigned logicAddr, 
                                  unsigned addr, Statistics* stats);
extern void printTable(PageTable* pt, char algorithm[]);

#endif