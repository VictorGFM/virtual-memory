#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "replacementalgorithms.h"

extern int isDebugMode;

void writePageFIFO(PageTable* pt, unsigned logicAddr, unsigned physicAddr, Statistics* stats) {
    if (pt->lastPageIndex == pt->tableSize) {
        pt->lastPageIndex = 0;
        pt->isFirstIteration = 0;
    }
    
    if(pt->pages[pt->lastPageIndex].dirtyPage) {
        stats->dirtyPagesWrittenDisk++;
        if(isDebugMode) printf(":::Dirty Page Written On Disk (%d): %x\n", 
                                stats->dirtyPagesWrittenDisk, 
                                pt->pages[pt->lastPageIndex].logicAddr);
    } else {
        pt->pages[pt->lastPageIndex].dirtyPage = 1;
    }

    pt->pages[pt->lastPageIndex].logicAddr = logicAddr;
    pt->pages[pt->lastPageIndex].physicAddr = physicAddr;

    pt->lastPageIndex++;
}

void writePageLRU(PageTable* pt, unsigned logicAddr, unsigned physicAddr,  Statistics* stats) {
    if(pt->lastPageIndex == pt->tableSize) {
        pt->isFirstIteration = 0;
        unsigned leastRecentlyUsedTime = UINT_MAX, leastRecentlyUsedIndex = 0;
        for (int i = 0; i < pt->tableSize; i++) {
            if (pt->pages[i].timeLastAccess < leastRecentlyUsedTime) {
                leastRecentlyUsedTime = pt->pages[i].timeLastAccess;
                leastRecentlyUsedIndex = i;
            }
        }

        if(pt->pages[leastRecentlyUsedIndex].dirtyPage) {
            stats->dirtyPagesWrittenDisk++;
            if(isDebugMode) printf(":::Dirty Page Written On Disk (%d): %x\n", 
                                    stats->dirtyPagesWrittenDisk, 
                                    pt->pages[leastRecentlyUsedIndex].logicAddr);
        }

        pt->pages[leastRecentlyUsedIndex].logicAddr = logicAddr;
        pt->pages[leastRecentlyUsedIndex].physicAddr = physicAddr;
        pt->pages[leastRecentlyUsedIndex].timeLastAccess = clock(); 
    } else {
        pt->pages[pt->lastPageIndex].logicAddr = logicAddr;
        pt->pages[pt->lastPageIndex].physicAddr = physicAddr;
        pt->pages[pt->lastPageIndex].dirtyPage = 1;
        pt->pages[pt->lastPageIndex].timeLastAccess = clock();

        pt->lastPageIndex++;
    }
}

void writePageSECONDCHANCE(PageTable* pt, unsigned logicAddr, unsigned physicAddr,
                           Statistics* stats) {
       
    if (pt->lastPageIndex == pt->tableSize) {
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
            pt->pages[pageIndex].logicAddr = logicAddr;
            pt->pages[pageIndex].physicAddr = physicAddr;
            break;
        }

        if (pt->pages[pageIndex].referenceBit == 1) {
            pt->pages[pageIndex].referenceBit = 0;
        }
    }

    pt->lastPageIndex = pageIndex + 1;
}

int isAlgorithmValid(char algorithm[]) {
    return strcmp(algorithm, LRU) == 0 || strcmp(algorithm, SECONDCHANCE) == 0 || strcmp(algorithm, FIFO) == 0 || strcmp(algorithm, CUSTOM) == 0;
}

int isFirstIteration(PageTable* pt) {
    return pt->isFirstIteration;
}

Statistics* newStatistics() {
    Statistics* stats = malloc(sizeof(Statistics));
    stats->pageSize = 0, stats->memSize = 0, stats->readPages = 0, stats->writtenPages = 0, 
    stats->pageFaults = 0, stats->dirtyPagesWrittenDisk = 0, stats->accessCount = 0;
    return stats;
} 

PageTable* newPageTable(unsigned memSize, unsigned pageSize) {
    PageTable* pt = malloc(sizeof(PageTable) + sizeof(MemPage) * (memSize/pageSize));
    pt->tableSize = memSize/pageSize, pt->pageSize = pageSize, pt->lastPageIndex = 0, 
    pt->isFirstIteration = 1;
    return pt;
}

MemPage* getPage(PageTable* pt, unsigned pageAddr) {
    for (int i = 0; i < pt->tableSize; i++) {
        if (pt->pages[i].logicAddr == pageAddr) {
            return &pt->pages[i];
        }
    }
    return NULL;
}

void updatePageByAlgorithm(PageTable* pt, MemPage* page, unsigned logicAddr, 
                           unsigned addr,  Statistics* stats) {
    unsigned isPageFault = (page == NULL);
    if(strcmp(stats->algorithm, FIFO) == 0) {
        if(isPageFault) {
            stats->pageFaults++;
            if(isDebugMode) printf(":::Page Fault (%d)\n", stats->pageFaults);
            writePageFIFO(pt, logicAddr, addr, stats);
        }
    } else if(strcmp(stats->algorithm, LRU) == 0) {
        if(isPageFault) {
            stats->pageFaults++;
            if(isDebugMode) printf(":::Page Fault (%d)\n", stats->pageFaults);
            writePageLRU(pt, logicAddr, addr, stats);
        } else {
            page->timeLastAccess=clock();
        }
    } else if(strcmp(stats->algorithm, SECONDCHANCE) == 0) {
        if(isPageFault) {
            stats->pageFaults++;
            if(isDebugMode) printf(":::Page Fault (%d)\n", stats->pageFaults);
            writePageSECONDCHANCE(pt, logicAddr, addr, stats);
        } else {
            page->referenceBit=1;
        }
    } else if (strcmp(stats->algorithm, CUSTOM) == 0) {
        writePageLRU(pt, logicAddr, addr, stats); //TODO: create Custom algorithm
    }
}

void printTable(PageTable* pt, char algorithm[]) {
    for (int i = 0; i < pt->tableSize; i++) {
        printf("Page %x - Address %x - Dirty Page %d", pt->pages[i].logicAddr, 
               pt->pages[i].physicAddr, pt->pages[i].dirtyPage);
        if (strcmp(algorithm, LRU) == 0) {
            printf(" - Time of Last Access %d\n", pt->pages[i].timeLastAccess);
            continue;
        }
        if (strcmp(algorithm, SECONDCHANCE) == 0) {
            printf(" - Reference Bit %d\n", pt->pages[i].referenceBit);
            continue;
        }
        printf("\n");
    }
}