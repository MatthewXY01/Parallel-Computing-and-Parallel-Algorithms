#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <math.h>
#define mpitype MPI_INT
typedef int dtype;
int N;
int K;
int print_flag;
void matGene(dtype**, int);
void printMat(dtype**, int);
void convolution(dtype**, int, dtype**, int, dtype*, int); 
int main(int argc, char *argv[]){
    int id;
    int p;
    double elapsed_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    if(argc != 4){
        if(!id) printf("Command line: %s <matrix size> <kernel size> <print_flag>", argv[0]);
        MPI_Finalize(); exit(1);
    }
    N = atoi(argv[1]);
    K = atoi(argv[2]);
    print_flag = strcmp("true", argv[3]);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    dtype** inMat;
    dtype* kernel;
    dtype** outMat;
    dtype** localInMat;
    dtype** localOutMat;

    int parts;
    parts = (int)(sqrt((double)p));
    if(parts*parts!=p){
        if(!id){
            printf("The number of processor should be perfect sqaure!\n");
        }
        MPI_Finalize(); exit(1);
    }

    int realoutMatLength = (N-K+0)/1+1;
    int outMatLength = (realoutMatLength%parts)==0 ? realoutMatLength: (realoutMatLength-realoutMatLength%parts+parts);
    int suboutMatLength = outMatLength/parts;
    int subinMatLength = suboutMatLength+K-1;
    int inMatLength = (subinMatLength-1)*parts+1;

    

    kernel = (dtype*)calloc(K*K, sizeof(dtype));

    localInMat = (dtype**)calloc(subinMatLength, sizeof(dtype*));
    localOutMat = (dtype**)calloc(suboutMatLength, sizeof(dtype*));
    
    for(int i = 0; i<subinMatLength; i++){
        localInMat[i] = (dtype*)calloc(subinMatLength, sizeof(dtype));
    }
    for(int i = 0; i<suboutMatLength; i++){
        localOutMat[i] = (dtype*)calloc(suboutMatLength, sizeof(dtype));
    }
    if(!id){
        
        inMat = (dtype**)calloc(inMatLength, sizeof(dtype*));
        outMat = (dtype**)calloc(outMatLength, sizeof(dtype*));
        
        for(int i = 0; i<inMatLength; i++){
            inMat[i] = (dtype*)calloc(inMatLength, sizeof(dtype));
        }
        for(int i = 0; i<outMatLength; i++){
            outMat[i] = (dtype*)calloc(outMatLength, sizeof(dtype));
        }
        // generate the matrix
        matGene(inMat, N);
        if(print_flag==0){
            printf("matrix: \n");
            printMat(inMat, N);
        }
        for(int i = 0; i<subinMatLength; i++){
            memcpy(localInMat[i], inMat[i], subinMatLength*sizeof(dtype));
        }
        for(int i=1; i<p; i++){
            int beginRow = i/parts*suboutMatLength;
            int beginCol = (i%parts)*suboutMatLength;
            for(int j=beginRow; j<beginRow+subinMatLength; j++){
                MPI_Send(&(inMat[j][beginCol]), subinMatLength, mpitype, i, 0, MPI_COMM_WORLD);
                
            }
            
        }
        // generate kernel
        for(int i = 0; i<K*K; i++){
            kernel[i] = rand()%3;
        }
        if(print_flag==0){
            printf("kernel: \n");
            for(int i = 0; i<K; i++){
                for(int j = 0; j<K; j++){
                    printf("%d ", kernel[i*K+j]);
                }
                putchar('\n');
            }
            putchar('\n');
        }
    }else{
        MPI_Status status;
        for(int i = 0; i<subinMatLength; i++){
            MPI_Recv(localInMat[i], subinMatLength, mpitype, 0, 0, MPI_COMM_WORLD, &status);
        }
    }
    
    MPI_Bcast(kernel, K*K, mpitype, 0, MPI_COMM_WORLD);    
    convolution(localInMat, subinMatLength, localOutMat, suboutMatLength, kernel, K);

    if(!id){
        MPI_Status status;
        for(int i = 0; i<suboutMatLength; i++){
            memcpy(outMat[i], localOutMat[i], suboutMatLength*sizeof(dtype));
        }
        for(int i = 1; i<p; i++){
            int beginRow = (i/parts)*suboutMatLength;
            int beginCol = (i%parts)*suboutMatLength;
            for(int j = beginRow; j<beginRow+suboutMatLength; j++){
                MPI_Recv(&(outMat[j][beginCol]), suboutMatLength, mpitype, i, 2, MPI_COMM_WORLD, &status);
            }
        }
        elapsed_time += MPI_Wtime();
        if(print_flag==0){
            printf("result: \n");
            printMat(outMat, outMatLength);
        }
        printf("Total elapsed time: %10.6f\n\n", elapsed_time);
    }else {
        for(int i = 0; i<suboutMatLength; i++){
            MPI_Send(localOutMat[i], suboutMatLength, mpitype, 0, 2, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    return 0;
}

void convolution(dtype** in, int inSize, dtype** out, int outSize, dtype* kernel, int kernelSize){
    for(int orow = 0; orow<outSize; orow++){
        for(int ocol = 0; ocol<outSize; ocol++){
            int irow, icol;
            for(int i = 0; i<kernelSize; i++){
                for(int j = 0; j<kernelSize; j++){
                    irow = i+orow;
                    icol = j+ocol;
                    out[orow][ocol]+=kernel[i*kernelSize+j]*in[irow][icol];
                }
            }
        }
    }
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
                printf("%d ", mat[i][j]);
            }
            putchar('\n');
        }
        putchar('\n');
    }
}