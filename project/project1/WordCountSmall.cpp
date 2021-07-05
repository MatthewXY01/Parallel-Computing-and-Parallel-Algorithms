#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <mpi.h>
#include <cmath>
#include <ctime>
#include <sys/time.h>
#include <dirent.h>
#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n)-1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id)+1, p, n) - BLOCK_LOW((id), p, n))
#define BLOCK_OWNER(index, p, n) (((p)*(index)+1)-1)/(n))
using namespace std;


int main(int argc, char *argv[]){
    int id;
    int p;
    double elapsed_time;
    string dirname;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if(argc!=2){
        if (!id) printf ("Command line: %s <address of the file>\n", argv[0]);
        MPI_Finalize(); exit (1);
    }
    dirname = string(argv[1]);

    /* seprators */
    char* seps = new char[128](); // enough
    int sepNum = 0;
    for(int i = 32; i<127; i++){
        if(!((i>='A'&&i<='Z')||(i>='a'&&i<='z'))){
            seps[sepNum++] = '\0'+i;
        }
    }
    seps[sepNum++] = '\n';
    seps[sepNum++] = '\t';
    seps[sepNum++] = '\r';
    seps[sepNum] = '\0';

    /* get filename list*/
    vector<string> files;
    DIR *dp;
    struct dirent *dirp;
    if((dp=opendir((dirname.c_str())))==NULL){
        cout<<"Can't open "<<dirname<<endl;
        closedir(dp);
        MPI_Finalize(); exit(1);
    }
    while((dirp = readdir(dp))!=NULL){
        if((strcmp(dirp->d_name, ".") != 0) && (strcmp(dirp->d_name, "..") != 0))
        files.push_back((dirp->d_name));
    }
    closedir(dp);

    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    
    sort(files.begin(), files.end());

    /* read files */
    ostringstream *osses = new ostringstream[p];
    string *mapResults  = new string [p];
    int *mapResultsLength = new int [p];
    hash<string> hash_str;

    int fileNumTotal = files.size();
    int fileNumLocal = BLOCK_SIZE(id, p, fileNumTotal);
    int fileBegin = BLOCK_LOW(id, p, fileNumTotal);
    char *token;
    for(int f=fileBegin; f<fileBegin+fileNumLocal; f++){
        int ossID;
        string filename = dirname+"/"+files[f];
        ifstream ifs(filename,  std::ios::binary);
        string line;
        while(getline(ifs, line)){
            if(line.empty()){
                continue;
            }
            token = strtok((char*)line.c_str(), seps);
            while(token!=NULL){
                ossID = hash_str(string(token))%p;
                osses[ossID]<<string(token);
                osses[ossID]<<'\n';
                token = strtok(NULL, seps);
            }
        }
    }
    for(int i = 0; i<p; i++){
        mapResults[i] = osses[i].str();
        mapResultsLength[i] = mapResults[i].length();
    }

    /* exchange among the workers */
    int *recvCountMap = new int[p]();
    int *displsMap = new int[p]();
    for(int i = 0; i<p; i++){
        MPI_Gather(&mapResultsLength[i], 1, MPI_INT, recvCountMap, 1, MPI_INT, i ,MPI_COMM_WORLD);
    }

    /* calculate displsMap */
    displsMap[0] = 0;
    int recvNumMap = recvCountMap[0]; 
    for(int i = 1; i<p; i++){
        recvNumMap+=recvCountMap[i];
        displsMap[i] = recvCountMap[i-1]+displsMap[i-1];
    }

    /* allocate memory for recv part of the map results for the reduce phase */
    char* recvBufMap = new char [recvNumMap+1]();
    for(int i = 0; i<p; i++){
        MPI_Gatherv(mapResults[i].data(), mapResultsLength[i], MPI_CHAR, recvBufMap, recvCountMap, displsMap, MPI_CHAR, i, MPI_COMM_WORLD);
    }

    /* reduce phase */
    map<string, int> count;
    string reduceInput = string(recvBufMap);
    token = strtok((char *)reduceInput.c_str(), "\n");
    while(token!=NULL){
        count[(string)token]++;
        token = strtok(NULL, "\n");
    }
    map<string, int>::iterator it = count.begin();

    /* merge to the root */
    ostringstream reduceResultsStream;
    for(it = count.begin(); it!=count.end(); it++){
        reduceResultsStream<<it->first<<' '<<it->second<<'\n';
    }
    string reduceResults = reduceResultsStream.str();

    int reduceResultsLength = reduceResults.length();
    int *recvCountReduce = new int[p]();
    int *displsReduce = new int[p]();
    MPI_Gather(&reduceResultsLength, 1, MPI_INT, recvCountReduce, 1, MPI_INT, 0, MPI_COMM_WORLD);
    char* recvBufReduce;
    int recvNumReduce;
    if(!id){
        displsReduce[0] = 0;
        recvNumReduce = recvCountReduce[0];
        for(int i = 1; i<p; i++){
            recvNumReduce+=recvCountReduce[i];
            displsReduce[i] = displsReduce[i-1]+recvCountReduce[i-1];
        }
        recvBufReduce = new char [recvNumReduce+1]();
    }
    MPI_Gatherv(reduceResults.data(), reduceResultsLength, MPI_CHAR, recvBufReduce, recvCountReduce, displsReduce, MPI_CHAR, 0, MPI_COMM_WORLD);
    if(!id){
        ofstream ofs("WordCountSmall_results.txt",  std::ios::binary);
        ofs<<recvBufReduce;
        ofs.close();
        elapsed_time+=MPI_Wtime();
        printf("Total elapsed time: %10.6f\n\n", elapsed_time);
    }
    MPI_Finalize();
    return 0;
}