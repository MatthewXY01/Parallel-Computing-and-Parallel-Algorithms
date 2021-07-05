#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

int main(int argc, char *argv[]){
    int id;
    int p;
    int array_len = 0;
    double myfn_exe_time;
    int* test_array;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if (argc != 2){
        if (!id) printf ("Command line: %s <m>\n", argv[0]);
        MPI_Finalize(); exit (1);
    }
    array_len = atoi(argv[1]);
    test_array =(int*)calloc(array_len, sizeof(int));
    int MyBcast(void*, int, MPI_Datatype, int, MPI_Comm);

    MPI_Barrier(MPI_COMM_WORLD);
    myfn_exe_time = -MPI_Wtime();
    MPI_Bcast(test_array, array_len, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if(!id){
        myfn_exe_time += MPI_Wtime();
        printf ("MyBcast finished! Total elapsed time: %10.6f\n", myfn_exe_time);
    }

    free(test_array);
    MPI_Finalize();
    return 0;
}

int MyBcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
    int id;
    int p;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if(id==root){
        int i = 0;
        for(i=0; i<p; i++){
            if (i==id)continue;
            MPI_Send(buffer, count, datatype, i, 0, comm);
        }
        return 1;
    }
    MPI_Recv(buffer, count, datatype, root, 0, comm, MPI_STATUS_IGNORE);
    return 1;
}
