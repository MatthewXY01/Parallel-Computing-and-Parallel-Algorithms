#include <stdio.h>
#include <mpi.h>
#define MALLOC_ERROR -2
int MY_Allgather(int rank, int worldSize, const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int recvcount, MPI_Datatype recvtype, MPI_Comm comm);
int get_size(MPI_Datatype);