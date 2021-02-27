#include <stdio.h>
#include "replacementalgorithms.h"

void replaceByFIFO(pageTable* pt, unsigned logicAddr, unsigned physicAddr) {
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

memPage* readPage(pageTable* pt, unsigned pageAddr) {
    for (int i = 0; i < pt->tableOccupation; i++) {
        if (pt->pages[i].logicAddr == pageAddr) {
            pt->pages[i].lastUsed = 0;
            return &pt->pages[i];
        }
    }

    return 0;
}

void writePage(pageTable* pt, unsigned logicAddr, unsigned physicAddr) {
    if (pt->tableOccupation < pt->tableSize) {
        for (int i = 0; i < pt->tableOccupation; i++) {
            pt->pages[i].lastUsed++;
        }

        pt->pages[pt->tableOccupation].logicAddr = logicAddr;
        pt->pages[pt->tableOccupation].physicAddr = physicAddr;
        pt->pages[pt->tableOccupation].lastUsed = 0;
        pt->tableOccupation++;
        return;
    }

    replaceByFIFO(pt, logicAddr, physicAddr);
}

void printTable(pageTable* pt) {
    for (int i = 0; i < pt->tableSize; i++) {
        printf("Página %x - Endereço %x - Último acesso %d\n", pt->pages[i].logicAddr, pt->pages[i].physicAddr, pt->pages[i].lastUsed);
    }
}

int isTableFull(pageTable* pt) {
    return pt->tableSize == pt->tableOccupation;
}