#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "myallgather.h"
#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n)-1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id)+1, p, n) - BLOCK_LOW((id), p, n))
#define BLOCK_OWNER(index, p, n) (((p)*(index)+1)-1)/(n))
int main(int argc, char *argv[]){
    int id;
    int p;
    int length = atoi(argv[2]);
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if(argc!=3){
        if(!id){
            printf ("Command line: %s <api> <length>\n", argv[0]);
            MPI_Finalize();exit(1);
        }
    }
    if(!(strcmp(argv[1], "MPI_Allgather")||strcmp(argv[1], "MY_Allgather"))){
        if(!id){
            printf ("Invalid API: %s\n", argv[1]);
        }
        MPI_Finalize();exit(1);
    }
    if(length<2*p){
        printf("Length is too short!\n");
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    length = (length/p)*p;
    int localLength = BLOCK_SIZE(id, p, length);
    char* sendbuf = (char*)calloc(localLength, sizeof(char));
    char* globalResult = (char*)calloc((length+1), sizeof(char));
    globalResult[length] = '\0';
    sendbuf[0] = id%p+'A';
    sendbuf[localLength-1] = '\n';
    for(int i = 1; i<localLength-1; i++){
        // int flag = rand()%2;
        // switch(flag){
        //     case 0:
        //         sendbuf[i] = '0' + rand()%10;
        //     break;
        //     case 1:
        //         sendbuf[i] = 'a' + rand()%26;
        //     break;
        // }
        sendbuf[i] = '0'+id%10;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double elapsed_time;
    if(strcmp(argv[1], "MPI_Allgather")){
        elapsed_time = -MPI_Wtime();
        MPI_Allgather(sendbuf, localLength, MPI_CHAR, globalResult, localLength, MPI_CHAR, MPI_COMM_WORLD);
        elapsed_time+=MPI_Wtime();
    }else if(strcmp(argv[1], "MY_Allgather")){
        elapsed_time = -MPI_Wtime();
        MY_Allgather(id, p, sendbuf, localLength, MPI_CHAR, globalResult, localLength, MPI_CHAR, MPI_COMM_WORLD);
        elapsed_time+=MPI_Wtime();
    }

    if(!id){
        // printf("%s", globalResult);
        printf("Testing %s\n", argv[1]);
        printf("Total elapsed time: %10.6f\n", elapsed_time);
    }
    fflush(stdout);
    free(sendbuf);
    free(globalResult);
    MPI_Finalize();
    return 0;
}