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
    for(int i = 0; i < numberFrames; i++) {
        physMem->frames[i].isAllocated = FREE;
        physMem->frames[i].pageAddress = -1;
    }
    return physMem;
}

PageTable* newPageTable(unsigned offsetSize) {
    unsigned numberPages = pow(2, ADDRESS_BITS_SIZE-offsetSize);
    PageTable* pt = malloc(sizeof(PageTable) + sizeof(Page) * numberPages);
    pt->numberPages = numberPages;
    for(int pageAddress = 0; pageAddress < numberPages; pageAddress++) {
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

void printTable(PhysMem* physMem, PageTable* pt, char algorithm[]) {
    if (strcmp(algorithm, FIFO) == 0) {
        printf("%-20s%-20s%-22s%-12s\n", "Endereço Físico", "| Endereço Lógico", 
            "| Bit de Validação", "| Bit Sujo"); 
    } else if (strcmp(algorithm, LRU) == 0) {
        printf("%-20s%-20s%-22s%-12s%-20s\n", "Endereço Físico", "| Endereço Lógico", 
            "| Bit de Validação", "| Bit Sujo", "| Tempo do último acesso"); 
    } else if (strcmp(algorithm, SECONDCHANCE) == 0) {
        printf("%-20s%-20s%-22s%-12s%-20s\n", "Endereço Físico", "| Endereço Lógico", 
            "| Bit de Validação", "| Bit Sujo", "| Bit de Referência"); 
    }
    for(int frameAddress = 0; frameAddress< physMem->numberFrames; frameAddress++) {
        Frame frame = physMem->frames[frameAddress];
        if(frame.isAllocated) {
            Page page = pt->pages[frame.pageAddress];
            if (strcmp(algorithm, FIFO) == 0) {
                printf("%-18x| %-16x| %-18c| %-10d\n", frameAddress, frame.pageAddress, 
                        page.validationBit, page.dirtyPage); 
            } else if (strcmp(algorithm, LRU) == 0) {
                printf("%-18x| %-16x| %-18c| %-10d| %-20d\n", frameAddress, frame.pageAddress, 
                        page.validationBit, page.dirtyPage, page.timeLastAccess);
            } else if (strcmp(algorithm, SECONDCHANCE) == 0) {
                printf("%-18x| %-16x| %-18c| %-10d| %-20d\n", frameAddress, frame.pageAddress, 
                        page.validationBit, page.dirtyPage, page.referenceBit);
            }
        } else {
            if (strcmp(algorithm, FIFO) == 0) {
                printf("%-18x| %-16s| %-18s| %-10s\n", frameAddress, "-", "-", "-"); 
            } else if (strcmp(algorithm, LRU) == 0) {
                printf("%-18x| %-16s| %-18s| %-10s| %-20s\n", frameAddress, "-", "-", "-", "-");
            } else if (strcmp(algorithm, SECONDCHANCE) == 0) {
                printf("%-18x| %-16s| %-18s| %-10s| %-20s\n", frameAddress, "-", "-", "-", "-"); 
            }
        }
    }
}

void writePageFIFO(PageTable* pt, PhysMem* physMem, unsigned pageAddress, Statistics* stats) {
    if (physMem->lastFrameAddress == physMem->numberFrames) {
        physMem->lastFrameAddress = 0;
    }

    if(physMem->frames[physMem->lastFrameAddress].isAllocated == FREE) {
        physMem->frames[physMem->lastFrameAddress].isAllocated = ALLOCATED;
        physMem->frames[physMem->lastFrameAddress].pageAddress = pageAddress;
    } else {
        unsigned deallocatedPageAddress = physMem->frames[physMem->lastFrameAddress].pageAddress;
        if(isDebugMode) printf(":::Deallocated Page Address: %x\n", deallocatedPageAddress);
        pt->pages[deallocatedPageAddress].validationBit = INVALID;

        if(pt->pages[deallocatedPageAddress].dirtyPage) {
            stats->dirtyPagesWrittenDisk++;
            if(isDebugMode) {
                printf(":::Dirty Page Written On Disk (%d): %x\n", 
                    stats->dirtyPagesWrittenDisk, deallocatedPageAddress);
            }
            pt->pages[deallocatedPageAddress].dirtyPage = 0;
        }
        
        physMem->frames[physMem->lastFrameAddress].pageAddress = pageAddress;
    }

    pt->pages[pageAddress].physicAddr = physMem->lastFrameAddress;
    pt->pages[pageAddress].validationBit = VALID;

    physMem->lastFrameAddress++;
}

void writePageLRU(PageTable* pt, PhysMem* physMem, unsigned pageAddress, Statistics* stats) {
    if (physMem->lastFrameAddress < physMem->numberFrames) {
        physMem->frames[physMem->lastFrameAddress].isAllocated = ALLOCATED;
        physMem->frames[physMem->lastFrameAddress].pageAddress = pageAddress;
        pt->pages[pageAddress].physicAddr = physMem->lastFrameAddress;
        pt->pages[pageAddress].validationBit = VALID;
        pt->pages[pageAddress].timeLastAccess = clock();

        physMem->lastFrameAddress++;
        return;
    }

    unsigned leastRecentlyUsedTime = UINT_MAX, leastRecentlyUsedIndex = 0;
    for (int i = 0; i < physMem->numberFrames; i++) {
        if (pt->pages[physMem->frames[i].pageAddress].timeLastAccess < leastRecentlyUsedTime) {
            leastRecentlyUsedTime = pt->pages[physMem->frames[i].pageAddress].timeLastAccess;
            leastRecentlyUsedIndex = i;
        }
    }

    unsigned deallocatedPageAddress = physMem->frames[leastRecentlyUsedIndex].pageAddress;
    if(isDebugMode) printf(":::Deallocated Page Address: %x\n", deallocatedPageAddress);
    pt->pages[deallocatedPageAddress].validationBit = INVALID;

    if(pt->pages[deallocatedPageAddress].dirtyPage) {
        stats->dirtyPagesWrittenDisk++;
        if(isDebugMode) {
            printf(":::Dirty Page Written On Disk (%d): %x\n", 
                stats->dirtyPagesWrittenDisk, deallocatedPageAddress);
        }
        pt->pages[deallocatedPageAddress].dirtyPage = 0;
    }

    physMem->frames[leastRecentlyUsedIndex].pageAddress = pageAddress;
    pt->pages[pageAddress].physicAddr = leastRecentlyUsedIndex;
    pt->pages[pageAddress].validationBit = VALID;
    pt->pages[pageAddress].timeLastAccess = clock();
}

void writePageSECONDCHANCE(PageTable* pt, PhysMem* physMem, unsigned pageAddress, Statistics* stats) {
    if (physMem->lastFrameAddress == physMem->numberFrames) {
        physMem->lastFrameAddress = 0;
    }

    if(physMem->frames[physMem->lastFrameAddress].isAllocated == FREE) {
        physMem->frames[physMem->lastFrameAddress].isAllocated = ALLOCATED;
        physMem->frames[physMem->lastFrameAddress].pageAddress = pageAddress;
        pt->pages[pageAddress].physicAddr = physMem->lastFrameAddress;
        pt->pages[pageAddress].validationBit = VALID;
        pt->pages[pageAddress].referenceBit = 0;
        physMem->lastFrameAddress++;
    } else {
        unsigned frameIndex = physMem->lastFrameAddress;
        unsigned deallocatedPageAddress = 0;
        for (int i = 0; i <= physMem->numberFrames; i++, frameIndex++) {
            if (frameIndex >= physMem->numberFrames) {
                frameIndex = 0;
            }

            deallocatedPageAddress = physMem->frames[frameIndex].pageAddress;
            if (pt->pages[deallocatedPageAddress].referenceBit == 1) {
                pt->pages[deallocatedPageAddress].referenceBit = 0;
                continue;
            }

            if (pt->pages[deallocatedPageAddress].referenceBit == 0) {
                if(isDebugMode) printf(":::Deallocated Page Address: %x\n", deallocatedPageAddress);
                pt->pages[deallocatedPageAddress].validationBit = INVALID;

                if(pt->pages[deallocatedPageAddress].dirtyPage) {
                    stats->dirtyPagesWrittenDisk++;
                    if(isDebugMode) {
                        printf(":::Dirty Page Written On Disk (%d): %x\n", 
                            stats->dirtyPagesWrittenDisk, deallocatedPageAddress);
                    }
                    pt->pages[deallocatedPageAddress].dirtyPage = 0;
                }

                break;
            }
        }

        physMem->frames[frameIndex].pageAddress = pageAddress;
        pt->pages[pageAddress].physicAddr = frameIndex;
        pt->pages[pageAddress].validationBit = VALID;

        physMem->lastFrameAddress = frameIndex + 1;
    }
}

void updatePageByAlgorithm(PageTable* pageTable, unsigned pageAddress, PhysMem* physMem,
                           Statistics* stats) {

    unsigned isPageFault = (pageTable->pages[pageAddress].validationBit == INVALID);
    if(strcmp(stats->algorithm, FIFO) == 0) {
        if(isPageFault) {
            stats->pageFaults++;
            if(isDebugMode) {
                printf(":::Page Fault (%d)\n", stats->pageFaults);
            }
            writePageFIFO(pageTable, physMem, pageAddress, stats);
        }
    } else if(strcmp(stats->algorithm, LRU) == 0) {
        if(isPageFault) {
            stats->pageFaults++;
            if(isDebugMode) {
                printf(":::Page Fault (%d)\n", stats->pageFaults);
            }
            writePageLRU(pageTable, physMem, pageAddress, stats);
        } else {
            pageTable->pages[pageAddress].timeLastAccess = clock();
        }
    } else if(strcmp(stats->algorithm, SECONDCHANCE) == 0) {
        if(isPageFault) {
            stats->pageFaults++;
            if(isDebugMode) {
                printf(":::Page Fault (%d)\n", stats->pageFaults);
            }
            writePageSECONDCHANCE(pageTable, physMem, pageAddress, stats);
        } else {
            pageTable->pages[pageAddress].referenceBit = 1;
        }
    } /* else if (strcmp(stats->algorithm, CUSTOM) == 0) {
        writePageLRU(pageTable, logicAddr, addr, stats); //TODO: create Custom algorithm
    } */
}