#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
pthread_spinlock_t lock;
char sh[6];
void* Thread(void* pParams) 
{
	int counter = 1;
	while (1) 
	{
		if (counter % 2 == 1) 
		{
			sh[0] = 'H'; 
			sh[1] = 'e'; 
			sh[2] = 'l'; 
			sh[3] = 'l'; 
			sh[4] = 'o'; 
			sh[5] = '\0';
		}
		else
		{
			sh[0] = 'B';
			sh[1] = 'y';
			sh[2] = 'e';
			sh[3] = '_';
			sh[4] = 'u';
			sh[5] = '\0';
		}	
		pthread_spin_lock(&lock);
		counter++;
		pthread_spin_unlock(&lock);
		//printf("%s\n", sh);
	}
	return NULL;
}

int main(void) 
{
	pthread_t thread_id;
	pthread_create(&thread_id, NULL, &Thread, NULL);
	pthread_t thread_id2;
	pthread_create(&thread_id2, NULL, &Thread, NULL);
	int n = 100;
	while (--n >= 0)
	{
		printf("%s\n", sh);
	}
		
}

