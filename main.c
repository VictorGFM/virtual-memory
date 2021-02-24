#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bitsToIgnore(int pageSize) {
    int s = 0;
    while (pageSize > 1) {
        pageSize = pageSize>>1;
        s++;
    }
    return s;
}

int main(int argsCount, char *args[]) {
    if (argsCount < 5) {
        printf("Invalid args count.\n");
        return 0;
    }

    char algorithm[7];
    strcpy(algorithm, args[1]);
    if (strcmp(algorithm, "lru") != 0 && strcmp(algorithm, "2a") != 0 && strcmp(algorithm, "fifo") != 0 && strcmp(algorithm, "custom") != 0) {
        printf("Invalid algorithm.\n");
        return 0;
    }

    FILE* logFile = fopen(args[2], "r");
    if (logFile == NULL) {
        printf("Invalid file.\n");
        return 0;
    }

    int pageSize = atoi(args[3]);
    if (pageSize < 1) {
        printf("Invalid page size.\n");
        return 0;
    }

    int memSize = atoi(args[4]);
    if (memSize < 1) {
        printf("Invalid memory size.\n");
        return 0;
    }

    return 0;
}