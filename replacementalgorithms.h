#ifndef REPLACEMENTALGORITHMS_H
#define REPLACEMENTALGORITHMS_H

    struct memPage {
        int logicAddr;
        int physicAddr;
        int lastUsed;
    };
    typedef struct memPage memPage;

    struct pageTable {
        int tableSize;
        int pageSize;
        memPage pages[];
    };
    typedef struct pageTable pageTable;

    extern int replaceByFIFO();
    extern int replaceByLRU();
    extern int replaceBySecondChance();
    extern int replaceByCustom();
    extern memPage findPage(int logicAddr, pageTable pt);

#endif