#include <stdio.h>
#include <math.h>
#include <mpi.h>
#define TRUE 1
#define FALSE 0
#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n)-1)
int main(int argc, char* argv[]){
    int id;
    int p;
    int lb, rb; // left boundary and right boundary of the global range
    int low_value, high_value; // for local
    int local_count, global_count = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    if (argc != 3){
        if(!id) printf("To ensure you have input valid interval!");
        MPI_Finalize();
        exit(1);
    }

    int process(int, int, int);
    int isPrime(int);

    lb = atoi(argv[1]);
    rb = atoi(argv[2]);
    /* allocate primitve task*/
    low_value = lb + BLOCK_LOW(id, p, rb-lb+1);
    high_value = lb + BLOCK_HIGH(id, p, rb-lb+1);
    
    local_count = process(low_value, high_value, rb);
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (!id){
        printf("The number of times that the consecutive odd integers are both prime is %d.\n", global_count);
    }
    fflush(stdout);
    MPI_Finalize();
    return 0;
}

int isPrime(int num){
    int i=2;
    if (num<2)return FALSE; // not prime
    while(i<=(int)sqrt((double)(num))){
        if ((num%i)==0){
            return FALSE;
        }
        i++;
    }
    return TRUE;
}

int process(int low_value, int high_value, int rb){
    int i = ((low_value%2)==1) ? low_value : (low_value+1); /* start from an odd number */
    int count = 0;
    int preIsPrime = FALSE;

    while(i<=high_value){
        if (!isPrime(i)){
            preIsPrime = FALSE;
            i+=2;
            continue;
        }
        if (preIsPrime) count++;
        else preIsPrime = TRUE;
        i+=2;
    }
    if(i<=rb && isPrime(i) && preIsPrime)count++;
    return count;
}


