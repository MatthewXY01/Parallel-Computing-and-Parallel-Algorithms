#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include "myallgather.h"
int get_size(MPI_Datatype t)  
{  
    if (t == MPI_BYTE) return sizeof(char);  
    if (t == MPI_DOUBLE) return sizeof(double);  
    if (t == MPI_FLOAT) return sizeof(float);  
    if (t == MPI_INT) return sizeof(int); 
    if (t == MPI_CHAR) return sizeof(char); 
    printf("Error: Unrecognized argument to 'get_size'\n");  
    fflush(stdout);  
    MPI_Abort(MPI_COMM_WORLD, -1);
    return 0;
}
int MY_Allgather(int rank, int worldSize, const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm){
    MPI_Request* sendRequest = (MPI_Request*)calloc(worldSize, sizeof(MPI_Request));
    MPI_Request* recvRequest = (MPI_Request*)calloc(worldSize, sizeof(MPI_Request));
    MPI_Status* status = (MPI_Status*)calloc(worldSize, sizeof(MPI_Status));
    size_t typeSize  =get_size(sendtype);
    if(sendRequest==NULL || recvRequest==NULL){
        MPI_Abort(comm, MALLOC_ERROR);
    }
    memcpy(recvbuf+rank*recvcount*typeSize, sendbuf, sendcount*typeSize);
    for(int i = 0; i<worldSize; i++){
        if(i!=rank)
            MPI_Isend(sendbuf, sendcount, sendtype, i, 0, comm, &sendRequest[i]);
    }
    for(int i = 0; i<worldSize; i++){
        if(i!=rank)
            MPI_Irecv(recvbuf+i*recvcount*typeSize, recvcount, recvtype, i, 0, comm, &recvRequest[i]);
    }
    for(int i = 0; i<worldSize; i++){
        if(i!=rank){
            MPI_Wait(&sendRequest[i], &status[i]);
            MPI_Wait(&recvRequest[i], &status[i]);
        }
    }
    return 1;
}