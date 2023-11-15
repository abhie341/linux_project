#define _GNU_SOURCE 
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/syscall.h>

#define SIZE 16
#define LARGE_SIZE 10240

#define N 10 
long thread_running[N];
int option = 0;



static void *run_9(void *arg) {
	int arr[LARGE_SIZE];

	size_t job = *(size_t*)arg;
	thread_running[job] = syscall(__NR_gettid);
	printf("(run_9) thread: %zu, pid: %d, tid: %ld\n", job, getpid(), syscall(__NR_gettid));
	for(int i = 0; i < LARGE_SIZE; i++)
	{
		arr[i] += i;
	}
	printf("############### 1. run_9 thread ##############\n");
	sleep(30);
	
	printf("############### 2. run_9 thread ##############\n");
	//return NULL;
	exit(-1);
}


static void *run(void *arg) {

	size_t job = *(size_t*)arg;
	thread_running[job] = syscall(__NR_gettid);
	printf("1. (run) thread: %zu, pid: %d, tid: %ld\n", job, getpid(), syscall(__NR_gettid));
	sleep(30);
	printf("2. (run) thread: %zu, pid: %d, tid: %ld\n", job, getpid(), syscall(__NR_gettid));
	
	//return NULL;
	exit(-1);
}

int main(int argc, char *argv[]) {
	size_t jobs[N];
	pthread_t threads[N];
	int fd_1 = 0;
	int fd_2 = 0;
	char buf[SIZE];
	int val = 0;
	int ret = 0;
	char *b = 0;

	if(argc != 1)
        {
                printf("Format: $./executable\n");
                return -1;
        }

	//initially no thread is running
	for(int i = 0; i < N; i++)
	{
		thread_running[i] = 0;
	}


	printf("Creating threads\n");
	for (size_t i=0; i<N; ++i) {
		jobs[i] = i;
		if(i == 9)
		{
			pthread_create(threads+i, NULL, run_9, jobs+i);
			continue;
		}
		pthread_create(threads+i, NULL, run, jobs+i);
	}


	//wait until all threads are running
	for(int i = 0; i < N; i++)
	{
		while(!thread_running[i])
		{
			sleep(1);
		}
		printf("Thread %d is running (tid: %ld)\n", i, thread_running[i]);
	}

        //write to sysfs file
	printf("############### 1. parent thread ##############\n");
        fd_1 = open("/sys/kernel/cs614_sysfs/cs614_value", O_WRONLY);
        assert(fd_1 >= 0);
	printf("############### 2. parent thread ##############\n");

	printf("############### 3. parent thread ##############\n");
        ret = write(fd_1, "7", 1);
        assert(ret == 1);
	printf("############### 4. parent thread ##############\n");


        //read from char device
	printf("############### 5. parent thread ##############\n");
        fd_2 = open("/dev/cs614_device", O_RDONLY);
        assert(fd_2 >= 0);
	printf("############### 6. parent thread ##############\n");

	printf("############### 7. parent thread reading ##############\n");
	ret = read(fd_2, buf, SIZE);
	printf("ret: %d, buf read: %d\n", ret, atoi(buf));
	assert(ret > 0);
	printf("############### 8. parent thread ##############\n");

        close(fd_1);
        close(fd_2);

	printf("############### 9. parent thread ##############\n");
	if(atoi(buf) == thread_running[9])
	{
		printf("Testcase passed\n");
		exit(0);
	}
	printf("Testcase failed\n");

	//printf("Waiting for threads to join\n");
	//for (size_t i=0; i<N; ++i) {
	//	pthread_join(threads[i], (void**)&b);
	//}

	exit(-1);
}

