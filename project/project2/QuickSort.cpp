#include <iostream>
#include <iomanip>
#include <ctime>
#include <sys/time.h>
#include <cstring>
#include <omp.h>
using namespace std;
void swap(int &a, int &b){
    int temp = a;
    a = b;
    b = temp;
}
void QuickSort(int *&array, int len, int l, int r, int id){
    if(len <= 1)return;
    int pivot = array[len/2];
    int lptr = 0;
    int rptr = len-1;
    while(lptr<=rptr){
        while(array[lptr]<pivot)lptr++;
        while(array[rptr]>pivot)rptr--;
        if(lptr<=rptr)
        {
            swap(array[lptr], array[rptr]);
            lptr++;
            rptr--;
        }
    }
    int *subarray[] = {array, &(array[lptr])};
    int sublen[] = {rptr+1, len-lptr};
    #pragma omp task default(none) firstprivate(subarray, sublen) shared(std::cout) firstprivate(l, rptr)
    {
        QuickSort(subarray[0], sublen[0], l, l+rptr, omp_get_thread_num());
    }
    #pragma omp task default(none) firstprivate(subarray, sublen) shared(std::cout) firstprivate(l, lptr, r)
    {
        QuickSort(subarray[1], sublen[1], l+lptr, r, omp_get_thread_num());
    }
}

int main(int argc, char* argv[])
{
    if(argc!=4){
        cout<<"Command line: "<<argv[0]<<" <number of threads> <number of elements> <print_flag>"<<endl;
        return 0;
    }
    int tn = atoi(argv[1]);
    int num = atoi(argv[2]);
    bool print_flag = ((strcmp(argv[3], "true")==0) ? true : false);
    int *array = new int[num];
    for(int i = 0; i<num; i++){
        array[i] = rand()%(2*num); // needn't to care why %(2*num)
        if(print_flag){
            cout<<array[i]<<' ';
        }
    }
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    omp_set_num_threads(tn);
    #pragma omp parallel
    {
        #pragma omp single
        QuickSort(array, num, 0, num-1, omp_get_thread_num());
    }
    
    gettimeofday(&t2, NULL);
    if(print_flag){
        putchar('\n');
        for(int i = 0; i<num; i++){
            cout<<array[i]<<' ';
        }
        putchar('\n');
    }
    cout<<"The sorting takes "<<1.0*(t2.tv_sec-t1.tv_sec) + 1.0*(t2.tv_usec-t1.tv_usec)/1000000<<" s."<<endl;
 
    return 0;
}