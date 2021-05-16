#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <stack>
#define MAX_NODE_SIZE 100
#define MAX_MESSAGE_SIZE 1000
#define INF 999999
using namespace std;
typedef pair<int, int> pii;
vector<pii> graph[MAX_NODE_SIZE + 1]; //destination, cost
priority_queue<pii, vector<pii>, greater<pii> > pq; //cost, destination
int dist[MAX_NODE_SIZE + 1][MAX_NODE_SIZE + 1];
int nearest[MAX_NODE_SIZE + 1][MAX_NODE_SIZE + 1];
stack<int> path[MAX_NODE_SIZE + 1];
int nodeNum;

void exitArgsErr() {
    fprintf(stderr, "usage: linkstate topologyfile messagesfile changesfile\n");
    exit(1);
}

void exitInputErr() {
    fprintf(stderr, "Error: open input file.\n");
    exit(1);
}

void dijkstra() {
    fill(&dist[0][0], &dist[MAX_NODE_SIZE][MAX_NODE_SIZE + 1], INF);
    for (int u = 0; u < nodeNum; u++) {
        for (int v = 0; v < nodeNum; v++) {
            dist[u][u] = 0;
            nearest[u][u] = u;
            pq.push(pii(0, u));
            while (!pq.empty()) {
                int minCost = pq.top().first;
                int x = pq.top().second;
                pq.pop();
                for (int idx = 0; idx < graph[x].size(); idx++) {
                    pii y = graph[x][idx];
                    if (dist[u][y.first] < minCost) continue;
                    if (dist[u][y.first] <= dist[u][x] + y.second) continue;
                    dist[u][y.first] = dist[u][x] + y.second;
                    pq.push(pii(dist[u][y.first], y.first));
                    nearest[u][y.first] = x;
                }
            }
            int node = v;
            while (true) {
                if (nearest[u][node] == u) break;
                node = nearest[u][node];
            }
            printf("%d %d %d\n", v, node, dist[u][v]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        exitArgsErr();
    }

    FILE *read_fp, *write_fp, *read_fp2;

    read_fp = fopen(argv[1], "r");
    if (read_fp == NULL) {
        exitInputErr();
    }

    fscanf(read_fp, "%d", &nodeNum);
    //네트워크 구성
    for (int idx = 0; idx < nodeNum; idx++) {
        int v, u, w;
        fscanf(read_fp, "%d %d %d", &v, &u, &w);
        graph[v].push_back(pii(u, w));
        graph[u].push_back(pii(v, w));
    }

    dijkstra();

    printf("Complete. Output file written to output_ls.txt.");
    return 0;
}