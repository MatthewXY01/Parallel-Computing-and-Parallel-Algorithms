#include <stdio.h>
#include <math.h>
#include <mpi.h>
#define INTERVALS 1000000
#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n)-1)

int main(int argc, char* argv[]){
    int id;
    int p;
    int low_value, high_value;
    double local_area, global_area = 0;
    double elapsed_time = 0.0;
    MPI_Init(&argc, &argv);
    MPI_Barrier (MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    double process(int, int, int);

    /* allocate primitve task*/
    low_value = BLOCK_LOW(id, p, INTERVALS);
    high_value = BLOCK_HIGH(id, p, INTERVALS);
    
    local_area = process(low_value, high_value, INTERVALS);
    MPI_Reduce(&local_area, &global_area, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    elapsed_time += MPI_Wtime();
    if (!id){
        printf("Area is %13.11lf.\nWith the help of %d processors, the total computation takes %.6lf seconds\n", global_area, p, elapsed_time);
    }
    fflush(stdout);
    MPI_Finalize();
    return 0;
}

double process(int low_value, int high_value, int total){
    double ysum = 0.0;
    double xi;
    int i;

    for(i=low_value; i<=high_value; i++){
        xi = (1.0/total)*(i+0.5);
        ysum+=4.0/(1.0+xi*xi);
    }
    return ysum/total;
}


