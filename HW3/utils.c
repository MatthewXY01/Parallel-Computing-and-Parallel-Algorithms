#include <mpi.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include "utils.h"

void *my_malloc(int id, int bytes){
    void *buffer;
    if((buffer = malloc((size_t) bytes))==NULL){
        printf("Error: Malloc failed for process %d\n", id);
        fflush(stdout);
        MPI_Abort(MPI_COMM_WORLD, MALLOC_ERROR);
    }
    return buffer;
}

int get_size(MPI_Datatype t)  
{  
    if (t == MPI_BYTE) return sizeof(char);  
    if (t == MPI_DOUBLE) return sizeof(double);  
    if (t == MPI_FLOAT) return sizeof(float);  
    if (t == MPI_INT) return sizeof(int);  
    printf("Error: Unrecognized argument to 'get_size'\n");  
    fflush(stdout);  
    MPI_Abort(MPI_COMM_WORLD, TYPE_ERROR);
}

void print_submatrix(  
    void **a,           /* IN - Doubly subscripted array */  
    MPI_Datatype dtype, /* IN - Type of element */  
    int rows,           /* IN - Number of rows */  
    int cols)           /* IN - Number of columns */  
{  
    int i,j;  
    for (i=0; i<rows; ++i) {
        for (j=0; j<cols; ++j) {
            if (dtype == MPI_DOUBLE)  
                printf("%6.3f",((double **)a)[i][j]);  
            else {  
                if (dtype == MPI_FLOAT)  
                printf("%6.3f",((float **)a)[i][j]);  
                else  
                printf("%6d",((int **)a)[i][j]);  
            }
        }
        putchar('\n');  
    }
    return;  
}

void print_row_striped_matrix(  
    void **a,           /* IN - 2D array */  
    MPI_Datatype dtype, /* IN - Matrix element type */  
    int m,              /* IN - Number of matrix rows */  
    int n,              /* IN - Number of matrix columns */  
    MPI_Comm comm)      /* IN - Communicator */  
{
    int p;              /* Number of processes */  
    int id;             /* Process rank */  
    int datum_size;     /* Size of matrix element */  
    int local_rows;     /* Rows in this process */  
    int max_block_size; /* Most matrix rows held by any process */  
    void *bstorage;     /* Elements received from another process */  
    void **b;           /* 2D array indexing into 'bstorage' */  
    MPI_Status status;  /* Result of receive */  
    int prompt;         /* Dummy variable */  
    int i;  
    MPI_Comm_size(comm, &p);  
    MPI_Comm_rank(comm, &id);  
    datum_size = get_size(dtype);  
    local_rows = BLOCK_SIZE(id,p,n);  
    if (!id) {  
        print_submatrix(a, dtype, local_rows, n);  
        if (p >1) {
                max_block_size = BLOCK_SIZE(p-1,p,m);  
                bstorage = (void *)my_malloc(id,  
                    max_block_size*n*datum_size);  
                b = (void **)my_malloc(id,  
                    max_block_size*PTR_SIZE);  
                b[0] = bstorage;  
                for (i=1; i<max_block_size; ++i) {  
                    b[i] = b[i-1] + n * datum_size;  
            }
            for (i=1; i<p; ++i) {  
                /* Send a prompt message to process i 
                in the default communicator */  
                MPI_Send(&prompt, 1, MPI_INT, i,  
                PROMPT_MSG, MPI_COMM_WORLD);  
                /* Receive datum with tag 'RESPONSE_MSG' from 
                process i in the default communicator */  
                MPI_Recv(bstorage, BLOCK_SIZE(i,p,m)*n, dtype,  
                i, RESPONSE_MSG, MPI_COMM_WORLD, &status);  
                print_submatrix(b, dtype, BLOCK_SIZE(i,p,m), n);  
            }
            free(b);  
            free(bstorage);  
        }
        putchar('\n');  
    } else {  
        /* Receive a prompt message from process 0 
        in the default communicator */  
        MPI_Recv(&prompt, 1, MPI_INT, 0, PROMPT_MSG,  
        MPI_COMM_WORLD,&status);  
        /* Send datum with tag 'RESPONSE_MSG' to process 0 
        in the default communicator */  
        MPI_Send(*a, local_rows*n, dtype, 0, RESPONSE_MSG,  
        MPI_COMM_WORLD);  
    }  
    return;  
}



void read_row_striped_matrix (
    char        *s,        /* IN - File name */
    void      ***subs,     /* OUT - 2D submatrix indices */
    void       **storage,  /* OUT - Submatrix stored here */
    MPI_Datatype dtype,    /* IN - Matrix element type */
    int         *m,        /* OUT - Matrix rows */
    int         *n,        /* OUT - Matrix cols */
    MPI_Comm     comm)     /* IN - Communicator */
{
    int          datum_size;   /* Size of matrix element */
    int          i;
    int          id;           /* Process rank */
    FILE        *infileptr;    /* Input file pointer */
    int          local_rows;   /* Rows on this proc */
    void       **lptr;         /* Pointer into 'subs' */
    int          p;            /* Number of processes */
    void        *rptr;         /* Pointer into 'storage' */
    MPI_Status   status;       /* Result of receive */
    int          x;            /* Result of read */

    MPI_Comm_size (comm, &p);
    MPI_Comm_rank (comm, &id);
    datum_size = sizeof(dtype);

    /* Process p-1 opens file, reads size of matrix,
        and broadcasts matrix dimensions to other procs */

    if (id == (p-1)) {
        infileptr = fopen (s, "r");
        if (infileptr == NULL) *m = 0;
        else {
            fread (m, sizeof(int), 1, infileptr);
            fread (n, sizeof(int), 1, infileptr);
        }      
    }
    MPI_Bcast (m, 1, MPI_INT, p-1, comm);

    if (!(*m)) MPI_Abort (MPI_COMM_WORLD, OPEN_FILE_ERROR);

    MPI_Bcast (n, 1, MPI_INT, p-1, comm);

    local_rows = BLOCK_SIZE(id,p,*m);

    /* Dynamically allocate matrix. Allow double subscripting
        through 'a'. */

    *storage = (void *) my_malloc (id,
        local_rows * *n * datum_size);
    *subs = (void **) my_malloc (id, local_rows * PTR_SIZE);

    lptr = &(*subs[0]);
    rptr = *storage;
    for (i = 0; i < local_rows; i++) {
        *(lptr++)= (void *) rptr;
        rptr += *n * datum_size;
    }

    /* Process p-1 reads blocks of rows from file and
        sends each block to the correct destination process.
        The last block it keeps. */

    if (id == (p-1)) {
        for (i = 0; i < p-1; i++) {
            x = fread (*storage, datum_size,
            BLOCK_SIZE(i,p,*m) * *n, infileptr);
            MPI_Send (*storage, BLOCK_SIZE(i,p,*m) * *n, dtype,
            i, 0, comm);
        }
        x = fread (*storage, datum_size, local_rows * *n,
            infileptr);
        fclose (infileptr);
    } else
        MPI_Recv (*storage, local_rows * *n, dtype, p-1,
            0, comm, &status);
}