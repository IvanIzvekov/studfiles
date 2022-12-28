//#include "lab8.h"
#include <pthread.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

const int LENGTH = 100;

int numbers[LENGTH];
#define THREADS 2

struct bounds{
    int left;
    int right;
};
long sum=0;

int turn=0, flagReady[2]={1,1};

void EnterCriticalRegion(int threadId){
    int x=1;
    int y=1-threadId;
    __atomic_store(&flagReady[threadId],&x,__ATOMIC_RELAXED);
    __atomic_store(&turn,&y,__ATOMIC_RELAXED);
    while (turn==(1-threadId) && flagReady[1-threadId]);
}

void LeaveCriticalRegion(int threadId){
    int x=0;
    __atomic_store(&flagReady[threadId],&x,__ATOMIC_RELAXED);
}

void* thread_sum_0(void* arg){
    struct bounds b=*(struct bounds*)arg;
    for(int i=b.left;i<b.right;i++){
        EnterCriticalRegion(0);
        __atomic_add_fetch(&sum,numbers[i],__ATOMIC_RELAXED);
        //sum+=numbers[i];
        LeaveCriticalRegion(0);
        usleep(1);
    }
    pthread_exit(NULL);
}

void* thread_sum_1(void* arg){
    struct bounds b=*(struct bounds*)arg;
    for(int i=b.left;i<b.right;i++){
        EnterCriticalRegion(1);
        __atomic_add_fetch(&sum,numbers[i],__ATOMIC_RELAXED);
        //sum+=numbers[i];
        LeaveCriticalRegion(1);
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