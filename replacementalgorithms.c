#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "replacementalgorithms.h"

void writePageFIFO(pageTable* pt, unsigned logicAddr, unsigned physicAddr) {
    if (pt->lastPageIndex >= pt->tableSize) {
        pt->lastPageIndex = 0;
        pt->isFirstIteration = 0;
    }
    pt->pages[pt->lastPageIndex].logicAddr = logicAddr;
    pt->pages[pt->lastPageIndex].physicAddr = physicAddr;

    pt->lastPageIndex++;
}

void writePageLRU(pageTable* pt, unsigned logicAddr, unsigned physicAddr) {
    if (pt->lastPageIndex < pt->tableSize) {
        for (int i = 0; i < pt->lastPageIndex; i++) {
            pt->pages[i].lastUsed++;
        }

        pt->pages[pt->lastPageIndex].logicAddr = logicAddr;
        pt->pages[pt->lastPageIndex].physicAddr = physicAddr;
        pt->pages[pt->lastPageIndex].lastUsed = 0;
        pt->lastPageIndex++;
        return;
    }
    pt->isFirstIteration = 0;

    int maxLastUsed = 0, maxLastUsedIndex = 0;
    for (int i = 0; i < pt->tableSize; i++) {
        if (maxLastUsed < pt->pages[i].lastUsed) {
            maxLastUsed = pt->pages[i].lastUsed;
            maxLastUsedIndex = i;
        }
        pt->pages[i].lastUsed++;
    }

    pt->pages[maxLastUsedIndex].logicAddr = logicAddr;
    pt->pages[maxLastUsedIndex].physicAddr = physicAddr;
    pt->pages[maxLastUsedIndex].lastUsed = 0;
}

void writePageSECONDCHANCE(pageTable* pt, unsigned logicAddr, unsigned physicAddr) {
    if (pt->lastPageIndex >= pt->tableSize) {
        pt->lastPageIndex = 0;
        pt->isFirstIteration = 0;
    } else if (pt->isFirstIteration) {
        pt->pages[pt->lastPageIndex].logicAddr = logicAddr;
        pt->pages[pt->lastPageIndex].physicAddr = physicAddr;
        pt->pages[pt->lastPageIndex].referenceBit = 0;

        pt->lastPageIndex++;
        return;
    }

    int pageIndex = pt->lastPageIndex;
    for (int c = 0; c <= pt->tableSize; c++, pageIndex++) {
        if (pageIndex >= pt->tableSize) {
            pageIndex = 0;
        }

        if (pt->pages[pageIndex].referenceBit == 0) {
            pt->pages[pt->lastPageIndex].logicAddr = logicAddr;
            pt->pages[pt->lastPageIndex].physicAddr = physicAddr;
            break;
        }

        if (pt->pages[pageIndex].referenceBit == 1) {
            pt->pages[pt->lastPageIndex].referenceBit = 0;
        }
    }

    pt->lastPageIndex = pageIndex + 1;
}

int IsAlgorithmValid(char algorithm[]) {
    return strcmp(algorithm, LRU) == 0 || strcmp(algorithm, SECONDCHANCE) == 0 || strcmp(algorithm, FIFO) == 0 || strcmp(algorithm, CUSTOM) == 0;
}

int IsTableFull(pageTable* pt) {
    return pt->tableSize == pt->lastPageIndex;
}

int IsFirstIteration(pageTable* pt) {
    return pt->isFirstIteration;
}

pageTable* NewPageTable(unsigned memSize, unsigned pageSize) {
    pageTable* pt= malloc(sizeof(pageTable) + sizeof(memPage) * (memSize/pageSize));
    pt->tableSize = memSize/pageSize, pt->pageSize = pageSize, pt->lastPageIndex = 0, pt->isFirstIteration = 1;
    return pt;
}

memPage* GetPage(pageTable* pt, unsigned pageAddr) {
    for (int i = 0; i < pt->tableSize; i++) {
        if (pt->pages[i].logicAddr == pageAddr) {
            pt->pages[i].lastUsed = 0;
            pt->pages[i].referenceBit = 1;
            return &pt->pages[i];
        }
    }

    return 0;
}

void UpdatePageByAlgorithm(pageTable* pt, unsigned logicAddr, unsigned addr, char algorithm[]) {
    if (strcmp(algorithm, LRU) == 0) {
        writePageLRU(pt, logicAddr, addr);
        return;
    }
    if (strcmp(algorithm, SECONDCHANCE) == 0) {
        writePageSECONDCHANCE(pt, logicAddr, addr);
        return;
    }
    if (strcmp(algorithm, FIFO) == 0) {
        writePageFIFO(pt, logicAddr, addr);
        return;
    }
    if (strcmp(algorithm, CUSTOM) == 0) {
        writePageLRU(pt, logicAddr, addr);
    }
}

void PrintTable(pageTable* pt, char algorithm[]) {
    for (int i = 0; i < pt->tableSize; i++) {
        printf("Página %x - Endereço %x", pt->pages[i].logicAddr, pt->pages[i].physicAddr);
        if (strcmp(algorithm, LRU) == 0) {
            printf(" - Usada desde %d iterações\n", pt->pages[i].lastUsed);
            continue;
        }
        if (strcmp(algorithm, SECONDCHANCE) == 0) {
            printf(" - Bit de referência %d\n", pt->pages[i].referenceBit);
            continue;
        }
        printf("\n");
    }
}