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
int nodeNum;

void exitArgsErr() {
    fprintf(stderr, "usage: linkstate topologyfile messagesfile changesfile\n");
    exit(1);
}

void exitInputErr() {
    fprintf(stderr, "Error: open input file.\n");
    exit(1);
}

void dijkstra(FILE *write_fp) {
    fill(&dist[0][0], &dist[MAX_NODE_SIZE][MAX_NODE_SIZE + 1], INF);
    fill(&nearest[0][0], &nearest[MAX_NODE_SIZE][MAX_NODE_SIZE + 1], INF);
    for (int u = 0; u < nodeNum; u++) {
        sort(graph[u].begin(), graph[u].end());
        for (int v = 0; v < nodeNum; v++) {
            dist[u][u] = 0;
            nearest[u][u] = u;
            pq.push(pii(0, u));
            while (!pq.empty()) {
                int minCost = pq.top().first;
                int x = pq.top().second;
                pq.pop();
                for (size_t idx = 0; idx < graph[x].size(); idx++) {
                    pii y = graph[x][idx];
                    if (dist[u][y.first] < minCost) continue;
                    if (dist[u][y.first] <= dist[u][x] + y.second) continue;
                    dist[u][y.first] = dist[u][x] + y.second;
                    pq.push(pii(dist[u][y.first], y.first));
                    nearest[u][y.first] = x;
                }
            }
            if (dist[u][v] == INF) continue;
            int node = v;
            while (true) {
                if (nearest[u][node] == u) break;
                node = nearest[u][node];
            }
            fprintf(write_fp, "%d %d %d\n", v, node, dist[u][v]);
        }
        fprintf(write_fp, "\n");
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
        if (dist[node][dest] == INF) {
            fprintf(write_fp, "from %d to %d cost infinite hops unreachable message %s\n", node, dest, message);
        }
        else {
            fprintf(write_fp, "from %d to %d cost %d hops ", node, dest, dist[node][dest]);
            int curr = nearest[node][dest];
            stack<int> path;
            path.push(curr);
            while (true) {
                curr = nearest[node][curr];
                path.push(curr);
                if (curr == node) break;
            }
            while (!path.empty()) {
                fprintf(write_fp, "%d ", path.top());
                path.pop();
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

    read_fp = fopen(argv[1], "r");
    if (read_fp == NULL) {
        exitInputErr();
    }

    fscanf(read_fp, "%d", &nodeNum);
    //네트워크 구성
    while (!feof(read_fp)) {
        int v, u, w;
        fscanf(read_fp, "%d %d %d", &v, &u, &w);
        graph[v].push_back(pii(u, w));
        graph[u].push_back(pii(v, w));
    }

    write_fp = fopen("output_ls.txt", "w");   
    dijkstra(write_fp);

    read_fp = fopen(argv[2], "r");
    deliverMessages(read_fp, write_fp);

    read_fp2 = fopen(argv[3], "r");
    if (read_fp2 == NULL) {
        exitInputErr();
    }

    int u, v, cost;
    fscanf(read_fp2, "%d %d %d", &u, &v, &cost);
    while (!feof(read_fp2)) {
        graph[u].erase(remove(graph[u].begin(), graph[u].end(), pii(v, dist[u][v])), graph[u].end());
        graph[v].erase(remove(graph[v].begin(), graph[v].end(), pii(u, dist[v][u])), graph[v].end());

        if (cost != -999) {
            graph[u].push_back(pii(v, cost));
            graph[v].push_back(pii(u, cost));
        }

        dijkstra(write_fp);

        read_fp = fopen(argv[2], "r");
        deliverMessages(read_fp, write_fp);

        fscanf(read_fp2, "%d %d %d", &u, &v, &cost);
    }

    printf("Complete. Output file written to output_ls.txt.\n");
    fclose(read_fp);
    fclose(read_fp2);
    fclose(write_fp);
    return 0;
}