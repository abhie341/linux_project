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

#define SIZE 16
#define N 10 

int thread_running[N];
int write_done = 0;
int fd_1 = 0;
int fd_2 = 0;
char buf[SIZE];
int ret = 0;

static void *run(void *arg) {
	size_t job = *(size_t*)arg;
	thread_running[job] = 1;
	if(job == 1)
	{
		//wait until all threads are running
		for(int i = 0; i < N; i++)
		{
			while(!thread_running[i])
			{
				sleep(1);
			}
			printf("Thread %d is running\n", i);
		}

		//wait for write operation to complete
		while(!write_done);
		printf("write operation complete\n");

		//perform read operation
		fd_2 = open("/dev/cs614_device", O_RDONLY);
		assert(fd_2 >= 0);

		memset(buf, '\0', SIZE);
		ret = read(fd_2, buf, SIZE);
		printf("ret: %d, buf read: %d\n", ret, atoi(buf));
		assert(ret > 0);

		//cross-check the result
		if(atoi(buf) == N+1)
		{
			printf("Testcase passed\n");
			exit(0);
		}
		printf("Testcase failed\n");
		exit(-1);
	}
	else if(job == 2)
	{
		//write to sysfs file
		fd_1 = open("/sys/kernel/cs614_sysfs/cs614_value", O_WRONLY);
		assert(fd_1 >= 0);

		ret = write(fd_1, "5", 1);
		assert(ret == 1);

		write_done = 1;
	}
	printf("Job %zu. Sleeping for 30 secs.\n", job);
	sleep(30);
	exit(-1);
}

int main(int argc, char *argv[]) {
	size_t jobs[N];
	pthread_t threads[N];
	int val = 0;
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
		pthread_create(threads+i, NULL, run, jobs+i);
	}


	sleep(30);
	return -1;
}

