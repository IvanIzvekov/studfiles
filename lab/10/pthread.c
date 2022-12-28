#include "lab8.h"
#include <pthread.h>
int numbers[LENGTH];
#define THREADS 2
struct bounds{
    int left;
    int right;
};
long sum=0;

int turn=0, flagReady[2]={1,1};

void* thread_sum_0(void* arg)
{
    struct bounds b=*(struct bounds*)arg;
    for(int i=b.left;i<b.right;i++)
    {
        flagReady[0]=1;
        turn=1;
        while(turn==1 && flagReady[1]);
        sum+=numbers[i];
        flagReady[0]=0;
        usleep(1);
    }
    pthread_exit(NULL);
}

void* thread_sum_1(void* arg){
    struct bounds b=*(struct bounds*)arg;
    for(int i=b.left;i<b.right;i++){
        flagReady[1]=1;
        turn=0;
        while(turn==0 && flagReady[0]);
        sum+=numbers[i];
        flagReady[1]=0;
        usleep(1);
    }
    pthread_exit(NULL);
}

int main(int argc, char** argv){
    int cores=1;
    char c;
    while ((c=getopt(argc,argv,"c:"))!=-1)
    {
        switch (c)
        {
        case 'c':
            cores=atoi(optarg);
            break;
        
        default:
            perror("getopt");
            exit(EXIT_FAILURE);
        }
    }
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for(int i=0;i<cores;i++){
        CPU_SET(i,&cpuset);
    }
    for(int i=0;i<LENGTH;i++){
        numbers[i]=1;
    }
    pthread_t threads[THREADS];
    struct bounds b0;
    b0.left=0;
    b0.right=LENGTH/THREADS;
    pthread_create(&threads[0],NULL,thread_sum_0,&b0);

    struct bounds b1;
    b1.left=LENGTH/THREADS;
    b1.right=LENGTH;
    pthread_create(&threads[1],NULL,thread_sum_1,&b1);
    pthread_setaffinity_np(threads[0],sizeof(cpu_set_t),&cpuset);
    pthread_setaffinity_np(threads[1],sizeof(cpu_set_t),&cpuset);
    struct timespec start,finish;
    clock_gettime(CLOCK_REALTIME,&start);
    for(int i=0;i<THREADS;i++){
        pthread_join(threads[i],NULL);
    }
    clock_gettime(CLOCK_REALTIME,&finish);
    double result=(finish.tv_sec-start.tv_sec)+(finish.tv_nsec-start.tv_nsec)/1E9;
    printf("Pthread result: %fs, sum = %ld\n",result, sum);
}