#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#define MAX_NODE_SIZE 100
#define MAX_MESSAGE_SIZE 1000
using namespace std;
typedef pair<int, int> pii;
struct node {
    int next[MAX_NODE_SIZE];
    int cost[MAX_NODE_SIZE];
};
node tables[MAX_NODE_SIZE + 1];
vector<pii> graph[MAX_NODE_SIZE + 1]; //dest, cost
int nodeNum;

void exitArgsErr() {
    fprintf(stderr, "usage: distvec topologyfile messagesfile changesfile\n");
    exit(1);
}

void exitInputErr() {
    fprintf(stderr, "Error: open input file.\n");
    exit(1);
}

void updateRoutingTables() {
    bool hasConverged = false;
    while (!hasConverged) {
        hasConverged = true;
        for (int node = 0; node < nodeNum; node++) {
            tables[node].next[node] = node;
            tables[node].cost[node] = 0;
            for (size_t v = 0; v < graph[node].size(); v++) {
                int neighbor = graph[node][v].first;
                int baseCost = graph[node][v].second;
                for (int idx = 0; idx < nodeNum; idx++) {
                    if (node == idx) continue;
                    if (idx == neighbor) continue;
                    if (tables[node].next[idx] < 0) continue;
                    //기존에 연결되지 않음
                    if (tables[neighbor].next[idx] < 0) {
                        tables[neighbor].next[idx] = node;
                        tables[neighbor].cost[idx] = tables[node].cost[idx] + baseCost;
                        hasConverged = false;
                    }
                    //최소값 업데이트
                    else if (tables[neighbor].cost[idx] > tables[node].cost[idx] + baseCost) {
                        tables[neighbor].next[idx] = node;
                        tables[neighbor].cost[idx] = tables[node].cost[idx] + baseCost;
                        hasConverged = false;
                    }
                    else if (tables[neighbor].next[idx] == node && tables[neighbor].cost[idx] != tables[node].cost[idx] + baseCost) {
                        tables[neighbor].cost[idx] = tables[node].cost[idx] + baseCost;
                        hasConverged = false;
                    }
                }
            }
        }
    }
}

void writeTableOutput(FILE *fp) {
    for (int idx = 0; idx < nodeNum; idx++) {
        for (int v = 0; v < nodeNum; v++) {
            if (tables[idx].next[v] < 0) continue;
            fprintf(fp, "%d %d %d\n", v, tables[idx].next[v], tables[idx].cost[v]);
        }
        fprintf(fp, "\n");
    }
}

void deliverMessages(FILE *read_fp, FILE *write_fp) {
    int node, dest;
    char message[MAX_MESSAGE_SIZE + 1];
    fscanf(read_fp, "%d %d %[^\n]s", &node, &dest, message);
    while (!feof(read_fp)) {
        if (tables[node].next[dest] < 0) {
            fprintf(write_fp, "from %d to %d cost infinite hops unreachable message %s\n", node, dest, message);
            continue;
        }
        fprintf(write_fp, "from %d to %d cost %d hops ", node, dest, tables[node].cost[dest]);
        int curr = node;
        while (curr != dest) {
            fprintf(write_fp, "%d ", curr);
            curr = tables[curr].next[dest];
        }
        fprintf(write_fp, "message %s\n", message);
        fscanf(read_fp, "%d %d %[^\n]s", &node, &dest, message);
    }
    fprintf(write_fp, "\n");
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        exitArgsErr();
    }

    FILE *read_fp, *write_fp;

    read_fp = fopen(argv[1], "r");
    if (read_fp == NULL) {
        exitInputErr();
    }
    
    memset(tables, -1, sizeof(tables));
    fscanf(read_fp, "%d", &nodeNum);
    //네트워크 구성
    for (int idx = 0; idx < nodeNum; idx++) {
        int v, e, w;
        fscanf(read_fp, "%d %d %d", &v, &e, &w);
        graph[v].push_back(pii(e, w));
        graph[e].push_back(pii(v, w));
        tables[v].next[e] = e;
        tables[v].cost[e] = w;
        tables[e].next[v] = v;
        tables[e].cost[v] = w;
    }

    updateRoutingTables();

    write_fp = fopen("output_dv.txt", "w");   
    writeTableOutput(write_fp); 

    read_fp = fopen(argv[2], "r");
    if (read_fp == NULL) {
        exitInputErr();
    }

    deliverMessages(read_fp, write_fp);

    printf("Complete. Output file written to output_dv.txt.");
    return 0;
}