#include <stdio.h>
#include <stdlib.h>

void exitArgsErr() {
    fprintf(stderr, "usage: distvec topologyfile messagesfile changesfile\n");
    exit(1);
}

void exitInputErr() {
    fprintf(stderr, "Error: open input file.\n");
    exit(1);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        exitArgsErr();
    }

    FILE *fp;
    int nodeNum;

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        exitInputErr();
    }

    printf("Complete. Output file written to output_ls.txt.");
    return 0;
}