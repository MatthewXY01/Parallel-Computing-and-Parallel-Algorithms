#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

int main(int argc, char *argv[]){
    int id;
    int p;
    int array_len = 0;
    double mpi_exe_time;
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

    MPI_Barrier(MPI_COMM_WORLD);
    mpi_exe_time = -MPI_Wtime();
    MPI_Bcast(test_array, array_len, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    if(!id){
        mpi_exe_time += MPI_Wtime();
        printf ("MPI_Bcast finished! Total elapsed time: %10.6f\n", mpi_exe_time);
    }

    free(test_array);
    MPI_Finalize();
    return 0;
}


