#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#define mpitype MPI_INT
typedef int dtype;
int N;
int print_flag;
void matGene(dtype**, int);
void printMat(dtype**, int);
int main(int argc, char *argv[]){
    int id;
    int p;
    double elapsed_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if (argc != 3) {
        if (!id) printf ("Command line: %s <length of the matrix> <print_flag>\n", argv[0]);
        MPI_Finalize(); exit (1);
    }
    N = atoi(argv[1]);
    print_flag = strcmp("true", argv[2]);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    dtype** A;
    dtype** B;
    dtype** C;
    dtype* storA;
    dtype* storB;
    dtype* storC;
    dtype** partA;
    dtype** partB;
    dtype** partC;
    int parts;
    parts = (int)(sqrt((double)p));
    if(parts*parts!=p){
        if(!id){
            printf("The number of processor should be perfect sqaure!\n");
        }
        MPI_Finalize(); exit(1);
    }

    int edgeLength = (N%parts)==0 ? N : N-N%parts+parts;
    int subedgeLength = edgeLength/parts;
    partA = (dtype**)calloc(subedgeLength, sizeof(dtype*));
    partB = (dtype**)calloc(edgeLength, sizeof(dtype*));
    partC = (dtype**)calloc(subedgeLength, sizeof(dtype*));

    for(int i = 0; i<subedgeLength; i++){
        partA[i] = (dtype*)calloc(edgeLength, sizeof(dtype));
    }
    for(int i = 0; i<edgeLength; i++){
        partB[i] = (dtype*)calloc(subedgeLength, sizeof(dtype));
    }
    for(int i = 0; i<subedgeLength; i++){
        partC[i] = (dtype*)calloc(subedgeLength, sizeof(dtype));
    }
    if(!id){
        storA = (dtype*)calloc(edgeLength*edgeLength, sizeof(dtype));
        storB = (dtype*)calloc(edgeLength*edgeLength, sizeof(dtype));
        storC = (dtype*)calloc(edgeLength*edgeLength, sizeof(dtype));
        A = (dtype**)calloc(edgeLength, sizeof(dtype*));
        B = (dtype**)calloc(edgeLength, sizeof(dtype*));
        C = (dtype**)calloc(edgeLength, sizeof(dtype*));
        for(int i = 0; i<edgeLength; i++){
            A[i] = &(storA[i*edgeLength]);
            B[i] = &(storB[i*edgeLength]);
            C[i] = &(storC[i*edgeLength]);
        }
        matGene(A, edgeLength);
        matGene(B, edgeLength);
        if(print_flag==0){
            printf("A: \n");
            printMat(A, N);
            printf("B: \n");
            printMat(B, N);
        }


        for(int i = 0; i<subedgeLength; i++){
            memcpy(partA[i], A[i], edgeLength*sizeof(dtype));
            memcpy(partC[i], C[i], subedgeLength*sizeof(dtype));
        }
        for(int i = 0; i<edgeLength; i++){
            memcpy(partB[i], B[i], subedgeLength*sizeof(dtype));
        }

        for(int i = 1; i<p; i++){
            int beginRow = (i/parts)*subedgeLength;
            int beginCol = (i%parts)*subedgeLength;
            for(int j = beginRow; j<beginRow+subedgeLength; j++){
                MPI_Send(A[j], edgeLength, mpitype, i, 0, MPI_COMM_WORLD);
            }
            for(int j = 0; j<edgeLength; j++){
                MPI_Send(&(B[j][beginCol]), subedgeLength, mpitype, i, 1, MPI_COMM_WORLD);
            }
        }
    }else {
        MPI_Status status;
        for(int i = 0; i<subedgeLength; i++){
            MPI_Recv(partA[i], edgeLength, mpitype, 0, 0, MPI_COMM_WORLD, &status);
        }
        for(int i = 0; i<edgeLength; i++){
            MPI_Recv(partB[i], subedgeLength, mpitype, 0, 1, MPI_COMM_WORLD, &status);
        }
    }

    for(int i = 0; i<subedgeLength; i++){
        for(int k = 0; k<edgeLength; k++){
            for(int j = 0; j<subedgeLength; j++){
                partC[i][j]+=partA[i][k]*partB[k][j];
            }
        }
    }

    if(!id){
        MPI_Status status;
        for(int i = 0; i<subedgeLength; i++){
            memcpy(C[i], partC[i], subedgeLength*sizeof(dtype));
        }
        for(int i =1; i<p; i++){
            int beginRow = (i/parts)*subedgeLength;
            int beginCol = (i%parts)*subedgeLength;
            for(int j = beginRow; j<beginRow+subedgeLength; j++){
                MPI_Recv(&(C[j][beginCol]), subedgeLength, mpitype, i, 2, MPI_COMM_WORLD, &status);
            }
        }
        elapsed_time+=MPI_Wtime();
        if(print_flag==0){
            printf("C: \n");
            printMat(C, N);
        }
        printf("Total elapsed time: %10.6f\n\n", elapsed_time);
    }else {
        for(int i = 0; i<subedgeLength; i++){
            MPI_Send(partC[i], subedgeLength, mpitype, 0, 2, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();
    return 0;
}

void matGene(dtype **mat, int edgeLength){
    for(int i = 0; i<edgeLength; i++){
        for(int j = 0; j<edgeLength; j++){
            mat[i][j] = rand()%10;
        }
    }
}
void printMat(dtype** mat, int edgeLength){
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    if(!id){
        for(int i = 0; i<edgeLength; i++){
            for(int j = 0; j<edgeLength; j++){
                printf("%d\t", mat[i][j]);
            }
            putchar('\n');
        }
        putchar('\n');
    }
}