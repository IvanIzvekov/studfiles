#include <stdio.h>
#include <string>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <thread>

using namespace std;

bool thread_is_finish = false;

int sum1 = 0;
int sum2 = 0;

double wtime()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}


vector<int> v(1000);
void fill(vector<int> &v)
{
	for (auto& i : v)
		i = rand()%100;
}
//int sum = 0;

void* my_thread1(void*) 
{
	sum1 = 0;
	for (auto i:v)
		sum1 += i;
	cout << "thread_is_finish!\n";
}

void my_thread2()
{
	sum2 = 0;
	for (auto i : v)
		sum2 += i;
	cout << "thread_is_finish!\n";
}

int main() 
{
	fill(v);
	double start, end, t;
	pthread_t th_id;
	start = wtime();
	pthread_create(&th_id, NULL, &my_thread1, nullptr);
	pthread_join(th_id, nullptr);
	end = wtime();
	t = end - start;
	cout << "time = " << t << "\n";
	
	printf("sum = %d\n", sum1);
	start = wtime();
	std::thread th(my_thread2);
	th.join();
	end = wtime();
	t = end - start;
	cout << "time = " << t << "\n";
	
	printf("sum = %d\n", sum2);
	return 0;
}

