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
void terminate(
    int id,
    char *error_message)
{
    if(!id){
        printf("Error: %s\n", error_message);
        fflush(stdout);
    }
    MPI_Finalize();
    exit(-1);
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

void print_subvector(
    void *a,
    MPI_Datatype dtype,
    int n)
{
    int i;
    for(i = 0; i<n; i++){
        if(dtype==MPI_DOUBLE)
            printf("%6.3f", ((double *)a)[i]);
        else {
            if(dtype==MPI_FLOAT)
                printf("%6.3f", ((float *)a)[i]);
            else if (dtype==MPI_INT)
                printf("%6d", ((int *)a)[i]);
        }
    }
}

void print_replicated_vector(
    void *v,
    MPI_Datatype dtype,
    int n,
    MPI_Comm comm)
{
    int id;
    MPI_Comm_rank(comm, &id);
    if(!id){
        print_subvector(v, dtype, n);
        printf("\n\n");
    }
}
void print_col_striped_matrix(  
    void **a,           /* IN - 2D array */  
    MPI_Datatype dtype, /* IN - Matrix element type */  
    int m,              /* IN - Number of matrix rows */  
    int n,              /* IN - Number of matrix columns */  
    MPI_Comm comm)      /* IN - Communicator */  
{
    int p;              /* Number of processes */  
    int id;             /* Process rank */  
    int datum_size;     /* Size of matrix element */  
    int local_cols;     /* Columns in this process */

    void *buffer;
    void **b;           /* 2D array indexing into 'bstorage' */  
    MPI_Status status;  /* Result of receive */  
    int i, j;
    int *recv_count;
    int *recv_disp;

    MPI_Comm_size(comm, &p);  
    MPI_Comm_rank(comm, &id);  
    datum_size = get_size(dtype);  
    local_cols = BLOCK_SIZE(id,p,n);

    create_mixed_xfer_arrays(id, p, n, &recv_count, &recv_disp);

    if(!id){
        buffer = (void*)my_malloc(id, n*datum_size);
    }
    for(i = 0; i<m; i++){
        MPI_Gatherv(a[i], BLOCK_SIZE(id, p, n), dtype, buffer, recv_count, recv_disp, dtype, 0, MPI_COMM_WORLD);
        if(!id) {
            print_subvector(buffer, dtype, n);
            putchar('\n');
        }
    }
    free (recv_count);
    free (recv_disp);
    if(!id){
        free(buffer);
        putchar('\n');
    }
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


void create_mixed_xfer_arrays(
    int id,
    int p,
    int n,
    int **count,
    int **disp)
{
    int i;

    *count = (int*) my_malloc(id, p*sizeof(int));
    *disp = (int*) my_malloc(id, p*sizeof(int));

    (*count)[0] = BLOCK_SIZE(0, p, n);
    (*disp)[0] = 0;
    for(i = 1; i<p; i++){
        (*disp)[i] = (*disp)[i-1]+(*count)[i-1];
        (*count)[i] = BLOCK_SIZE(1, p, n);
    }
}


void read_col_striped_matrix (
    char *s,
    void ***subs,
    void **storage,
    MPI_Datatype dtype,
    int *m,
    int *n,
    MPI_Comm comm)
{
    void *buffer;
    int datum_size;
    int i, j;
    int id;
    FILE *infileptr;
    int local_cols;
    void **lptr;
    void *rptr;
    int p;
    int *send_count;
    int *send_disp;

    MPI_Comm_size(comm, &p);
    MPI_Comm_rank(comm, &id);
    datum_size = get_size(dtype);

    if(id==(p-1)){
        infileptr = fopen(s, "r");
        if(infileptr==NULL) *m = 0;
        else{
            fread(m, sizeof(int), 1, infileptr);
            fread(n, sizeof(int), 1, infileptr);
        }
    }

    MPI_Bcast(m, 1, MPI_INT, p-1, comm);
    if(!(*m))MPI_Abort(comm, OPEN_FILE_ERROR);
    MPI_Bcast(n, 1, MPI_INT, p-1, comm);
    local_cols = BLOCK_SIZE(id, p, *n);

    *storage = my_malloc(id, *m*local_cols*datum_size);
    *subs = (void**) my_malloc(id, *m*PTR_SIZE);
    lptr = (void**) *subs; 
    rptr = (void*) *storage;

    for(i = 0; i<*m; i++){
        *(lptr++) = (void*) rptr;
        rptr += local_cols * datum_size;
    }
    if(id==(p-1))
        buffer = my_malloc(id, *n*datum_size);
    
    create_mixed_xfer_arrays(id, p, *n, &send_count, &send_disp);
    for(i = 0; i<*m; i++){
        if(id==(p-1))
            fread(buffer, datum_size, *n, infileptr);
        MPI_Scatterv(buffer, send_count, send_disp, dtype, (*storage)+i*local_cols*datum_size, local_cols, dtype, p-1, comm);

    }
    free (send_count);
    free (send_disp);
    if(id==(p-1))free (buffer);
}

void read_replicated_vector(
    char *s,
    void **v,
    MPI_Datatype dtype,
    int *n,
    MPI_Comm comm)
{
    int datum_size;
    int i;
    int id;
    FILE *infileptr;
    int p;
    
    MPI_Comm_rank(comm, &id);
    MPI_Comm_size(comm, &p);
    datum_size = get_size(dtype);
    if(id==(p-1)){
        infileptr = fopen(s, "r");
        if(infileptr == NULL) *n = 0;
        else fread(n, sizeof(int), 1, infileptr);
    }

    MPI_Bcast(n, 1, MPI_INT, p-1, MPI_COMM_WORLD);
    if(!*n) terminate (id, "Cannot open vector file");

    *v = my_malloc(id, *n * datum_size);

    if(id==(p-1)){
        fread(*v, datum_size, *n, infileptr);
        fclose(infileptr);
    }
    MPI_Bcast(*v, *n, dtype, p-1, MPI_COMM_WORLD);
}