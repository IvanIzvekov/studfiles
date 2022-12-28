#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

char sh[6];
int error=0;

int turn=0, flagReady[2]={1,1};

void* thread_0(void* pParams){
    int counter=0;
    while(1){
        flagReady[0]=1;
        turn=1;
        while(turn==1 && flagReady[1]);
        if(counter%2){
            sh[0]='H';
            sh[1]='e';
            sh[2]='l';
            sh[3]='l';
            sh[4]='o';
            sh[5]='\0';
        }
        else{
            sh[0]='B';
            sh[1]='y';
            sh[2]='e';
            sh[3]='_';
            sh[4]='u';
            sh[5]='\0';
        }
        flagReady[0]=0;
        usleep(1);
        counter++;
    }
    return NULL;
}

void* thread_1(void* pParams){
    while(1){
        flagReady[1]=1;
        turn=0;
        while(turn==0 && flagReady[0]);
        printf("%s\n",sh);
        flagReady[1]=0;
        usleep(1);
    }
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
    pthread_t thread[2];
    pthread_create(&thread[0],NULL,thread_0,NULL);
    pthread_create(&thread[1],NULL,thread_1,NULL);
    pthread_setaffinity_np(thread[0],sizeof(cpu_set_t),&cpuset);
    pthread_setaffinity_np(thread[1],sizeof(cpu_set_t),&cpuset);
    pthread_join(thread[0],NULL);
    pthread_join(thread[1],NULL);
}