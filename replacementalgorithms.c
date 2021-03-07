#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "replacementalgorithms.h"

extern int isDebugMode;

Statistics* newStatistics() {
    Statistics* stats = malloc(sizeof(Statistics));
    stats->pageSize = 0, stats->memSize = 0, stats->readPages = 0, stats->writtenPages = 0, 
    stats->pageFaults = 0, stats->dirtyPagesWrittenDisk = 0, stats->accessCount = 0;
    return stats;
} 

PhysMem* newPhysicalMemory(unsigned memSize, unsigned pageSize) {
    unsigned numberFrames = memSize/pageSize;
    PhysMem* physMem = malloc(sizeof(PhysMem) + sizeof(Frame) * numberFrames);
    physMem->numberFrames = numberFrames, physMem->lastFrameAddress = 0;
    for(int i=0; i<numberFrames; i++) {
        physMem->frames[i].isAllocated = FREE;
        physMem->frames[i].pageAddress = -1;
    }
    return physMem;
}

PageTable* newPageTable(unsigned offsetSize) {
    unsigned numberPages = pow(2, ADDRESS_BITS_SIZE-offsetSize);
    PageTable* pt = malloc(sizeof(PageTable) + sizeof(Page) * numberPages);
    pt->numberPages = numberPages;
    for(int pageAddress=0; pageAddress<numberPages; pageAddress++) {
        pt->pages[pageAddress].physicAddr = -1;
        pt->pages[pageAddress].validationBit = INVALID;
        pt->pages[pageAddress].dirtyPage = 0;
        pt->pages[pageAddress].referenceBit = 0;
        pt->pages[pageAddress].timeLastAccess = 0;
    }
    return pt;
}

int isAlgorithmValid(char algorithm[]) {
    return strcmp(algorithm, LRU) == 0 || strcmp(algorithm, SECONDCHANCE) == 0 || strcmp(algorithm, FIFO) == 0 || strcmp(algorithm, CUSTOM) == 0;
}

void printTable(PageTable* pt, char algorithm[], int printOnlyValidPages) {
    for (int pageAddress = 0; pageAddress < pt->numberPages; pageAddress++) {
        if((printOnlyValidPages && pt->pages[pageAddress].validationBit == VALID)
           || !printOnlyValidPages) {
            printf("Page %x - Address %x - Validation Bit %c - Dirty Page %d", pageAddress, 
                pt->pages[pageAddress].physicAddr, pt->pages[pageAddress].validationBit, 
                pt->pages[pageAddress].dirtyPage);
            if (strcmp(algorithm, LRU) == 0) {
                printf(" - Time of Last Access %d\n", pt->pages[pageAddress].timeLastAccess);
                continue;
            }
            if (strcmp(algorithm, SECONDCHANCE) == 0) {
                printf(" - Reference Bit %d\n", pt->pages[pageAddress].referenceBit);
                continue;
            }
            printf("\n");
        } 
    }
}

void writePageFIFO(PageTable* pt, unsigned pageAddress, PhysMem* physMem, Statistics* stats) {
    if (physMem->lastFrameAddress == physMem->numberFrames) {
        physMem->lastFrameAddress = 0;
    }
    
    if(physMem->frames[physMem->lastFrameAddress].isAllocated == FREE) {
        physMem->frames[physMem->lastFrameAddress].isAllocated = ALLOCATED;
        physMem->frames[physMem->lastFrameAddress].pageAddress = pageAddress;
    } else {
        unsigned deallocatedPageAddress = physMem->frames[physMem->lastFrameAddress].pageAddress;
        pt->pages[deallocatedPageAddress].validationBit = INVALID;
        if(pt->pages[deallocatedPageAddress].dirtyPage) {
            stats->dirtyPagesWrittenDisk++;
            if(isDebugMode) {
                printf(":::Dirty Page Written On Disk (%d): %x\n", 
                        stats->dirtyPagesWrittenDisk, deallocatedPageAddress);
            }
        }
        pt->pages[deallocatedPageAddress].dirtyPage = 0;
        physMem->frames[physMem->lastFrameAddress].pageAddress = pageAddress;
    }

    pt->pages[pageAddress].physicAddr = physMem->lastFrameAddress;
    pt->pages[pageAddress].validationBit = VALID;

    physMem->lastFrameAddress++;
}

void writePageLRU(PageTable* pt, unsigned logicAddr, unsigned physicAddr,  Statistics* stats) {
    /* if(pt->lastPageIndex == pt->numberPages) {
        pt->isFirstIteration = 0;
        unsigned leastRecentlyUsedTime = UINT_MAX, leastRecentlyUsedIndex = 0;
        for (int i = 0; i < pt->numberPages; i++) {
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
    } */
}

void writePageSECONDCHANCE(PageTable* pt, unsigned logicAddr, unsigned physicAddr,
                           Statistics* stats) {
       
    /* if (pt->lastPageIndex == pt->numberPages) {
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
    for (int c = 0; c <= pt->numberPages; c++, pageIndex++) {
        if (pageIndex >= pt->numberPages) {
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

    pt->lastPageIndex = pageIndex + 1; */
}

void updatePageByAlgorithm(PageTable* pt, unsigned pageAddress, PhysMem* physMem,  
                           Statistics* stats) {
    unsigned isPageFault = (pt->pages[pageAddress].validationBit == INVALID);
    if(strcmp(stats->algorithm, FIFO) == 0) {
        if(isPageFault) {
            stats->pageFaults++;
            if(isDebugMode) printf(":::Page Fault (%d)\n", stats->pageFaults);
            writePageFIFO(pt, pageAddress, physMem, stats);
        }
    } 
    /* else if(strcmp(stats->algorithm, LRU) == 0) {
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
    } */
}