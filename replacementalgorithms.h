#ifndef REPLACEMENTALGORITHMS_H
#define REPLACEMENTALGORITHMS_H

    struct memPage { //TODO: confirmar formato e o que deve ser armazenado na tabela
        unsigned logicAddr;
        unsigned physicAddr;
        unsigned lastUsed;
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

    #define LRU "lru"
    #define FIFO "fifo"
    #define SECONDCHANCE "2a"
    #define CUSTOM "custom"

    void writePageLRU(pageTable* pt, unsigned logicAddr, unsigned physicAddr);
    void writePageFIFO(pageTable* pt, unsigned logicAddr, unsigned physicAddr);
    void writePageSECONDCHANCE(pageTable* pt, unsigned logicAddr, unsigned physicAddr);
    void writePageCUSTOM(pageTable* pt, unsigned logicAddr, unsigned physicAddr);

    extern int IsAlgorithmValid(char algorithm[]);
    extern int IsTableFull(pageTable* pt);
    extern int IsFirstIteration(pageTable* pt);
    extern pageTable* NewPageTable(unsigned memSize, unsigned pageSize);
    extern memPage* GetPage(pageTable* pt, unsigned pageAddr);
    extern void UpdatePageByAlgorithm(pageTable* pt, unsigned logicAddr, unsigned addr, char algorithm[]);
    extern void PrintTable(pageTable* pt, char algorithm[]);

#endif