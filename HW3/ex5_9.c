#include <stdio.h>
#include <math.h>
#include <mpi.h>
#define BLOCK_LOW(id, p, n) ((id)*(n)/(p))
#define BLOCK_HIGH(id, p, n) (BLOCK_LOW((id)+1, p, n)-1)
#define BLOCK_SIZE(id, p, n) (BLOCK_LOW((id)+1, p, n) - BLOCK_LOW((id), p, n))
#define BLOCK_OWNER(index, p, n) (((p)*(index)+1)-1)/(n))
#define TRUE 1
#define FALSE 0
#define MIN(a,b)  ((a)<(b)?(a):(b))
int isPrime(int);
int main(int argc, char *argv[]){
    int id;
    int p;
    int n;
    int size; // n-1
    int seed_count = 0, count = 0, global_count = 0;
    int i, j;
    char *marked, *global_marked;
    int *seed_primes;
    double elapsed_time;

    MPI_Init (&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();
    MPI_Comm_rank (MPI_COMM_WORLD, &id);
    MPI_Comm_size (MPI_COMM_WORLD, &p);
    if (argc != 2) {
        if (!id) printf ("Command line: %s <m>\n", argv[0]);
        MPI_Finalize(); exit (1);
    }
    n = atoi(argv[1]);
    size = n-1; // range from 2 to n, n-1 integers totally
    if (p > (int) sqrt((double) n)) {
        if (!id) printf ("Too many processes\n");
        MPI_Finalize();
        exit (1);
    }

    seed_primes = (int *) malloc((int)sqrt((double) n));
    marked = (char *) malloc (size);
    global_marked = (char *) malloc (size);
    if (marked == NULL||global_marked==NULL||seed_primes==NULL) {
        printf ("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit (1);
    }

   /* Initialize the marked */
    for (i = 0; i < size; i++){
        marked[i] = FALSE;
        global_marked[i]  =FALSE;
    }

    for (i = 2; i <=(int)sqrt((double) n); i++){
        if(isPrime(i)) seed_primes[count++] =i;
    }

    for(i = id; i<count; i+=p){
        int seed = seed_primes[i];
        int j = seed*2;
        for (;j<=n; j+=seed){
            marked[j-2] = TRUE;
        }
    }
    MPI_Reduce (marked, global_marked, size, MPI_CHAR, MPI_LOR, 0, MPI_COMM_WORLD);
    if(!id){
        for (i = 0; i<size; i++){
            if(global_marked[i]==FALSE){
                global_count++;
            }
        }
        printf("Finished!\n");
    }
    elapsed_time += MPI_Wtime();

    if(!id){
        printf("%d primes are less than or equal to %d\n", global_count, n);
        printf("Total elapsed time: %10.6f\n", elapsed_time);
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