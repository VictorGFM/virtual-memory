#ifndef REPLACEMENTALGORITHMS_H
#define REPLACEMENTALGORITHMS_H
#include <time.h>
#include <limits.h>
#include <math.h>
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

struct Page {
    unsigned physicAddr;
    char validationBit;
    unsigned dirtyPage;
    unsigned timeLastAccess;
    char referenceBit;
};
typedef struct Page Page;

struct PageTable {
    unsigned numberPages;
    Page pages[];
};
typedef struct PageTable PageTable;

struct Frame {
    int isAllocated;
    unsigned pageAddress;
};
typedef struct Frame Frame;

struct PhysMem {
    unsigned numberFrames;
    unsigned lastFrameAddress;
    Frame frames[];
};
typedef struct PhysMem PhysMem;

void writePageFIFO(PageTable* pt, PhysMem* physMem, unsigned pageAddress, Statistics* stats);
void writePageLRU(PageTable* pt, PhysMem* physMem, unsigned pageAddress, Statistics* stats);
void writePageSECONDCHANCE(PageTable* pt, PhysMem* physMem, unsigned pageAddress, Statistics* stats);
void writePageCUSTOM(PageTable* pt, PhysMem* physMem, unsigned pageAddress, Statistics* stats);

extern Statistics* newStatistics();
extern PhysMem* newPhysicalMemory(unsigned memSize, unsigned pageSize);
extern PageTable* newPageTable(unsigned offsetSize);
extern int isAlgorithmValid(char algorithm[]);
extern void printTable(PhysMem* physMem, PageTable* pt, char algorithm[]);
extern void updatePageByAlgorithm(PageTable* pt, unsigned pageAddress, PhysMem* physMem, 
                                  Statistics* stats);

#endif