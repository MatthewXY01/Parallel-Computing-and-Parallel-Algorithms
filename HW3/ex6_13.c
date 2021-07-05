#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <string.h>
#include "utils.h"

#define MPI_TYPE MPI_INT
#define ALIVE 1
#define DEAD 0
typedef int dtype;
int main(int argc, char *argv[]){
    int id;
    int p;
    int block_size, datum_size;
    int i, j, k;
    int m, n;
    dtype** a; /* Doubly-subscripted array */
    dtype* storage; /* Local portion of array elements */
    double elapsed_time;
    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if (argc != 4){
        if (!id) printf ("Command line: %s <m>\n", argv[0]);
        MPI_Finalize(); exit (1);
    }
    j = atoi(argv[2]);
    k = atoi(argv[3]);
    if (j<=0||k<=0||k>j){
        if(!id) printf ("Invalid arguments!\n");
        MPI_Finalize();exit(1);
    }
    void play(int, int, int, int, int, int, int, dtype**);
    read_row_striped_matrix (argv[1], (void ***)&a, (void **)&storage, MPI_TYPE, &m, &n, MPI_COMM_WORLD);
    if(p>m){
        if(!id) printf("Too many processors!\n");
        MPI_Finalize(); exit(1);
    }
    block_size = BLOCK_SIZE(id, p, m);
    datum_size = get_size(MPI_TYPE);
    if(!id){
        printf("Initial state:\n");
    }
    print_row_striped_matrix((void**) a , MPI_TYPE, m, n, MPI_COMM_WORLD);

    play(id, p, m, n, block_size, j, k, a);

    if(!id){
        elapsed_time += MPI_Wtime();
        printf ("Finished! Total elapsed time: %10.6f\n", elapsed_time);
        printf("Final state:\n");
    }
    print_row_striped_matrix((void **)a, MPI_TYPE, m, n, MPI_COMM_WORLD);

    MPI_Finalize();
}

dtype getNewStatus(int r, int c, int max_r, int max_c, dtype* boundAbove, dtype* boundBelow, dtype** sub){
    dtype * ptr;
    int nAlive = 0;
    int dr, dc;
    int tmp_r, tmp_c;
    for(dr = -1; dr<=1; dr++){
        tmp_r = r+dr;
        if(tmp_r <0)ptr = boundAbove;
        else if(tmp_r==max_r)ptr = boundBelow;
        else ptr = sub[tmp_r];
        for(dc = -1; dc<=1; dc++){
            tmp_c = c+dc;
            if(tmp_c<0||tmp_c>=max_c)continue;
            if(ptr[tmp_c]==ALIVE)nAlive++;
        }
    }
    nAlive-=sub[r][c];
    if(sub[r][c]==ALIVE){
        if(nAlive==2||nAlive==3)return ALIVE;
        return DEAD;
    }else{
        if(nAlive==3)return ALIVE;
        return DEAD;
    }
}

void play(int id, int p, int m, int n, int block_size, int num_itr, int print_freq, dtype** a){
    int i;
    int datum_size = get_size(MPI_TYPE);
    dtype* tmp_storage;
    dtype** tmp_sub;
    /* temp space for the change, used to update the submatrix at the end of the iteration. */
    tmp_storage = (dtype*) malloc(block_size*n*datum_size);
    tmp_sub = (dtype**)malloc(block_size*sizeof(dtype*));
    if(tmp_storage==NULL||tmp_sub==NULL){
        printf("Error: Malloc failed for process %d\n", id);
        fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }
    tmp_sub[0] = tmp_storage;
    for (i=1; i<block_size; ++i){
        tmp_sub[i] = tmp_sub[i-1] + n;
    }
    /* array for receiving the border lines from neighour processors */
    dtype *boundAbove, *boundBelow;
    boundAbove = (dtype*)malloc((size_t)(n*datum_size));
    boundBelow = (dtype*)malloc((size_t)(n*datum_size));
    /* send before recv */
    if(id>0){
        MPI_Send(a[0], n, MPI_TYPE, id-1, DATA_MSG, MPI_COMM_WORLD);
    }
    if(id<p-1){
        MPI_Send(a[block_size-1], n, MPI_TYPE, id+1, DATA_MSG, MPI_COMM_WORLD);
    }
    /* iterative update */
    dtype getNewStatus(int, int, int, int, dtype*, dtype*, dtype**);
    int itr = 0;
    int r, c;
    while(1){
        if(id!=0)MPI_Recv(boundAbove, n, MPI_TYPE, id-1, DATA_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        else memset(boundAbove, 0, n*datum_size);
        
        if(id!=(p-1))MPI_Recv(boundBelow, n, MPI_TYPE, id+1, DATA_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        else memset(boundBelow, 0, n*datum_size);

        for(r = 0; r<block_size; r++){
            for(c = 0; c<n; c++){
                tmp_sub[r][c] = getNewStatus(r, c, block_size, n, boundAbove, boundBelow, a);
            }
        }
        for(r = 0; r<block_size; r++){
            for(c = 0; c<n; c++){
                a[r][c] = tmp_sub[r][c];
            }
        }
        itr++;
        if(itr%print_freq==0){
            if(!id)printf("After %d iteration(s):\n", itr);
            print_row_striped_matrix((void **)a, MPI_TYPE, m, n, MPI_COMM_WORLD);
        }
        if(itr==num_itr)break; // Done!

        if(id>0){
            MPI_Send(a[0], n, MPI_TYPE, id-1, DATA_MSG, MPI_COMM_WORLD);
        }
        if(id<p-1){
            MPI_Send(a[block_size-1], n, MPI_TYPE, id+1, DATA_MSG, MPI_COMM_WORLD);
        }
    }
    free(tmp_storage);
    free(tmp_sub);
}




