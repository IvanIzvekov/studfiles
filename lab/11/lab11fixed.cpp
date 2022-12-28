#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
using namespace std;

int r1, items = 0;

sem_t notEmpty, notFull;

void* produce(void* arg)
{
	while (1) {
		sem_wait(&notFull);
		sleep(rand() % 100 * 0.01);
		cout << "Producer produces Present = " << ++items << endl;
		sem_post(&notEmpty);
		sleep(rand() % 100 * 0.01);
	}
}


void* consume(void* arg)
{
	while (1) {
		sem_wait(&notEmpty);
		sleep(rand() % 100 * 0.01);
		cout << "Consumer consumes Present = " << --items << endl;
		sem_post(&notFull);
		sleep(rand() % 100 * 0.01);
	}
}

int main(int argv, char* argc[])
{

	int N;
	cout << "Enter the capacity of the buffer";
	cin >> N;

	
	pthread_t producer, consumer;

	
	pthread_attr_t attr;

	
	sem_init(&notEmpty, 0, 0);
	sem_init(&notFull, 0, N);

	
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	
	r1 = pthread_create(&producer, &attr, produce, NULL);
	if (r1) {
		cout << "Error in creating thread" << endl;
		exit(-1);
	}

	r1 = pthread_create(&consumer, &attr, consume, NULL);
	if (r1) {
		cout << "Error in creating thread" << endl;
		exit(-1);
	}

	
	pthread_attr_destroy(&attr);

	
	r1 = pthread_join(producer, NULL);

	if (r1) {
		cout << "Error in joining thread" << endl;
		exit(-1);
	}

	r1 = pthread_join(consumer, NULL);
	if (r1) {
		cout << "Error in joining thread" << endl;
		exit(-1);
	}
	
	pthread_exit(NULL);

	return 0;
}

