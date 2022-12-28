#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

char sh[6];
int error=0;

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

void* thread_0(void* pParams){
    int counter=0;
    while(1){
        EnterCriticalRegion(0);
        if(counter%2){
            sh[0]='H';sh[1]='e';sh[2]='l';sh[3]='l';sh[4]='o';sh[5]='\0';
        }
        else{
            sh[0]='B';sh[1]='y';sh[2]='e';sh[3]='_';sh[4]='u';sh[5]='\0';
        }
        LeaveCriticalRegion(0);
        usleep(1);
        counter++;
    }
    return NULL;
}

int main(int argc, char** argv){
    pthread_t thread_id;
    pthread_create(&thread_id,NULL,thread_0,NULL);
    while(1){
        EnterCriticalRegion(1);
        printf("%s\n",sh);
        LeaveCriticalRegion(1);
        usleep(1);
    }
}