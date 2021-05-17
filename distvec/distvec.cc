#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <algorithm>
#define MAX_NODE_SIZE 100
#define MAX_MESSAGE_SIZE 1000
using namespace std;
struct node {
    int next[MAX_NODE_SIZE];
    int cost[MAX_NODE_SIZE];
};
node tables[MAX_NODE_SIZE + 1];
int graph[MAX_NODE_SIZE + 1][MAX_NODE_SIZE + 1]; //dest, cost
int tmpGraph[MAX_NODE_SIZE + 1][MAX_NODE_SIZE + 1];
node tmpTables[MAX_NODE_SIZE + 1];
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
            for (int neighbor = 0; neighbor < nodeNum; neighbor++) {
                if (neighbor == node) continue; //자기 자신 제외
                if (graph[node][neighbor] == -1) continue; //이웃x
                int baseCost = graph[node][neighbor];
                for (int u = 0; u < nodeNum; u++) {
                    if (node == u) continue;
                    if (u == neighbor) continue;
                    if (tables[node].next[u] == -1) continue; //전달해줄 정보x
                    //기존에 연결되지 않음
                    if (tables[neighbor].next[u] < 0) {
                        tables[neighbor].next[u] = node;
                        tables[neighbor].cost[u] = tables[node].cost[u] + baseCost;
                        hasConverged = false;
                    }
                    //최소값 업데이트
                    else if (tables[neighbor].cost[u] > tables[node].cost[u] + baseCost) {
                        tables[neighbor].next[u] = node;
                        tables[neighbor].cost[u] = tables[node].cost[u] + baseCost;
                        hasConverged = false;
                    }
                    //경로의 next hop에 해당하는 node로부터의 정보는 무조건 수용
                    else if (tables[neighbor].next[u] == node && tables[neighbor].cost[u] != tables[node].cost[u] + baseCost) {
                        tables[neighbor].cost[u] = tables[node].cost[u] + baseCost;
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
    if (read_fp == NULL) {
        exitInputErr();
    }
    int node, dest;
    char message[MAX_MESSAGE_SIZE + 1];
    fscanf(read_fp, "%d %d %[^\n]s", &node, &dest, message);
    while (!feof(read_fp)) {
        if (tables[node].next[dest] < 0) {
            fprintf(write_fp, "from %d to %d cost infinite hops unreachable message %s\n", node, dest, message);
        }
        else {
            fprintf(write_fp, "from %d to %d cost %d hops ", node, dest, tables[node].cost[dest]);
            int curr = node;
            while (curr != dest) {
                fprintf(write_fp, "%d ", curr);
                curr = tables[curr].next[dest];
            }
            fprintf(write_fp, "message %s\n", message);
        }
        fscanf(read_fp, "%d %d %[^\n]s", &node, &dest, message);
    }
    fprintf(write_fp, "\n");
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        exitArgsErr();
    }

    FILE *read_fp, *write_fp, *read_fp2;

    memset(tables, -1, sizeof(tables));
    memset(graph, -1, sizeof(graph));

    read_fp = fopen(argv[1], "r");
    if (read_fp == NULL) {
        exitInputErr();
    }
    fscanf(read_fp, "%d", &nodeNum);
    //네트워크 구성
    while (!feof(read_fp)) {
        int v, u, w;
        fscanf(read_fp, "%d %d %d", &v, &u, &w);
        graph[v][u] = w;
        graph[u][v] = w;
        tables[v].next[u] = u;
        tables[v].cost[u] = w;
        tables[u].next[v] = v;
        tables[u].cost[v] = w;
    }

    memcpy(&tmpGraph, &graph, sizeof(graph));
    memcpy(&tmpTables, &tables, sizeof(tables));

    updateRoutingTables();

    write_fp = fopen("output_dv.txt", "w");   
    writeTableOutput(write_fp); 

    read_fp = fopen(argv[2], "r");
    deliverMessages(read_fp, write_fp);

    read_fp2 = fopen(argv[3], "r");
    if (read_fp2 == NULL) {
        exitInputErr();
    }

    int u, v, cost;
    fscanf(read_fp2, "%d %d %d", &u, &v, &cost);
    while (!feof(read_fp2)) {
        //routing table 새로 구성 위해 prev state로 초기화
        memcpy(&graph, &tmpGraph, sizeof(tmpGraph));
        memcpy(&tables, &tmpTables, sizeof(tmpTables));

        if (cost == -999) {
            graph[v][u] = graph[u][v] = -1;
            tables[v].next[u] = tables[v].cost[u] = tables[u].next[v] = tables[u].cost[v] = -1;
        }
        else {
            graph[v][u] = graph[u][v] = cost; 
            tables[v].next[u] = u;
            tables[v].cost[u] = cost;
            tables[u].next[v] = v;
            tables[u].cost[v] = cost;
        }

        memcpy(&tmpGraph, &graph, sizeof(graph));
        memcpy(&tmpTables, &tables, sizeof(tables));

        updateRoutingTables();
        writeTableOutput(write_fp);
        read_fp = fopen(argv[2], "r");
        deliverMessages(read_fp, write_fp);

        fscanf(read_fp2, "%d %d %d", &u, &v, &cost);
    }

    printf("Complete. Output file written to output_dv.txt.\n");
    fclose(read_fp);
    fclose(read_fp2);
    fclose(write_fp);
    return 0;
}