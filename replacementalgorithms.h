#ifndef REPLACEMENTALGORITHMS_H
#define REPLACEMENTALGORITHMS_H

    struct memPage { //TODO: confirmar formato e o que deve ser armazenado na tabela
        unsigned logicAddr;
        unsigned physicAddr;
        unsigned lastUsed;
    };
    typedef struct memPage memPage;

    struct pageTable {
        unsigned tableSize;
        unsigned pageSize;
        unsigned tableOccupation;
        memPage pages[];
    };
    typedef struct pageTable pageTable;

    extern void replaceByFIFO(pageTable* pt, unsigned logicAddr, unsigned physicAddr);
    extern void replaceByLRU(); //wip
    extern void replaceBySecondChance(); //wip
    extern void replaceByCustom(); //wip
    extern int isTableFull(pageTable* pt);
    extern memPage* readPage(pageTable* pt, unsigned pageAddr);
    extern void writePage(pageTable* pt, unsigned logicAddr, unsigned physicAddr);
    extern void printTable(pageTable* pt);

#endif