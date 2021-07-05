#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <ctime>
#include <sys/time.h>
#include <cstring>
#include <omp.h>
#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n)-1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id)+1, p, n) - BLOCK_LOW((id), p, n))
#define BLOCK_OWNER(index, p, n) (((p)*(index)+1)-1)/(n))
using namespace std;

typedef struct Graph{
    float *rank;
    int **outEdge;
    int **inEdge;
    int *outDegree;
    int *inDegree;
    int vertexNum;
}Graph;

Graph graphGene(int vertexNum, int maxEdge = 10){
    // utilize container to sample non-repetitive random numbers
    vector<int> temp;
    for (int i = 0; i<vertexNum; i++){
        temp.push_back(i);
    }
    random_shuffle(temp.begin(), temp.end());
    Graph graph{new float[vertexNum], new int*[vertexNum], new int*[vertexNum], new int[vertexNum](), new int[vertexNum](), vertexNum};
    for(int i = 0; i<vertexNum; i++){
        graph.inDegree[i] = rand()%maxEdge+1; // random number of neibours that point to the current node
        int randomBegin  = rand()%(vertexNum-graph.inDegree[i]+1);
        graph.inEdge[i] = new int[graph.inDegree[i]];
        graph.rank[i] = 1.0;
        for(int j = 0; j<graph.inDegree[i]; j++){
            graph.inEdge[i][j] = temp[j+randomBegin];
            graph.outDegree[temp[j+randomBegin]]++;
        }
    }
    return graph;
}
void processSubgraph(Graph &g, float *tempRank, int threads, int tid){
    int begin  = BLOCK_LOW(tid, threads, g.vertexNum);
    int size = BLOCK_SIZE(tid, threads, g.vertexNum);
    for(int i = begin; i<begin+size; i++){
        int neighbour;
        tempRank[i] = 0.0;
        for(int j = 0; j<g.inDegree[i]; j++){
            neighbour = g.inEdge[i][j];
            tempRank[i]+=g.rank[neighbour]/g.outDegree[neighbour];
        }
    }
}

void PageRank(Graph &g, int iterations, int threads){
    float *tempRank = new float[g.vertexNum]();
    for(int t = 0; t<iterations; t++){
        #pragma omp parallel
        {
            #pragma omp for
            for(int tid = 0; tid<threads; tid++){
                processSubgraph(g, tempRank, threads, tid);
            }
            #pragma omp for
            for(int i = 0; i<g.vertexNum; i++){
                g.rank[i] = tempRank[i];
            }
        }
    }
}

int main(int argc, char* argv[]){
    if(argc!=6){
        cout<<"Command line: "<<argv[0]<<" <number of threads>\n<number of vertices>\n<max value of edges per vertex>\n<number of iterations>\n<print_flag>"<<endl;
        return 0;
    }
    int tn = atoi(argv[1]);
    int numVertex = atoi(argv[2]);
    int maxEdge = atoi(argv[3]);
    int iterations = atoi(argv[4]);
    bool print_flag = ((strcmp(argv[5], "true")==0) ? true: false);
    Graph g = graphGene(numVertex, maxEdge);
    if(print_flag){
        for(int i = 0; i<g.vertexNum; i++){
            cout<<"node id: "<<i<<" inDegree: "<<g.inDegree[i]<<" outDegree: "<<g.outDegree[i]<<" rank: "<<g.rank[i]<<" pointed from ";
            for(int j = 0; j<g.inDegree[i]; j++){
                cout<<g.inEdge[i][j]<<' ';
            }
            putchar('\n');
        }
    }
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    // time_t t1 = time(0);
    PageRank(g, iterations, tn);
    gettimeofday(&t2, NULL);
    // time_t t2  = time(0);
    if(print_flag){
        cout<<"After "<<iterations<<" iterations:"<<endl;
        for(int i = 0; i<g.vertexNum; i++){
            cout<<"node id: "<<i<<" rank: "<<g.rank[i]<<endl;
        }
    }
    cout<<"The calculation takes "<<1.0*(t2.tv_sec-t1.tv_sec) + 1.0*(t2.tv_usec-t1.tv_usec)/1000000<<" s."<<endl;
    return 0;
}
