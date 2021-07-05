#include <iostream>
#include <iomanip>
#include <ctime>
#include <sys/time.h>
#include <omp.h>

using namespace std;

int main(int argc, char* argv[])
{
    if(argc!=3){
        cout<<"Command line: "<<argv[0]<<" <number of threads> <number of trials>"<<endl;
        return 0;
    }
    int tn = atoi(argv[1]);
    double x = 0, y = 0;
    int total = atoi(argv[2]); // times of trails
    int count = 0;
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    #pragma omp parallel  private(t2) num_threads(tn)
    {
        int test;
        gettimeofday(&t1, NULL);
        unsigned int seed = omp_get_thread_num();
        #pragma omp for firstprivate(x, y) reduction(+:count)
        for(int i = 0; i<total; i++)
        {
            x = (double)(rand_r(&seed))/RAND_MAX;
            y = (double)(rand_r(&seed))/RAND_MAX;
            if((x-0.5)*(x-0.5)+(y-0.5)*(y-0.5)<0.25)count++;
        }
        gettimeofday(&t2, NULL);
        #pragma omp critical
        {
        cout<<"Thread "<<omp_get_thread_num()<<" takes "<<setprecision(5)<<1.0*(t2.tv_sec-t1.tv_sec) + 1.0*(t2.tv_usec-t1.tv_usec)/1000000<<" s."<<endl;
        }
    }
    double pi = 4.0*count/total;
    gettimeofday(&t2, NULL);
    cout<<"After the calculation, total time cost: "<<1.0*(t2.tv_sec-t1.tv_sec) + 1.0*(t2.tv_usec-t1.tv_usec)/1000000<<" s."<<endl;
    printf("After %d trials, Pi is estimated to be %f\n", total, pi);
    return 0;
}