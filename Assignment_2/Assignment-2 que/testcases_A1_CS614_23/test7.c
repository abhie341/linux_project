#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define SIZE 16 
#define LEN 2
#define NUM_OPS 5

int main(int argc, char* argv[])
{
	int fd_1 = 0;
	int fd_2 = 0;
	char buf[SIZE];
	int val = 0;
	int ret = 0;
	int pid = 0;
        struct rusage usage;
	char operation[LEN];
	int wait_status = 0;
	int num_fork = 999;
	int rand_num = 0;

	srand(time(NULL));

	if(argc != 1)
	{
		printf("Format: $./executable\n");
		return -1;
	}

	//write to sysfs file
	fd_1 = open("/sys/kernel/cs614_sysfs/cs614_value", O_WRONLY);
	assert(fd_1 >= 0);

	memset(operation, '\0', LEN);
	operation[0] = '0' + (getpid() % NUM_OPS);

	ret = write(fd_1, operation, 1);
	assert(ret == 1);
	printf("Root process, pid: %d, ppid: %d, num_fork: %d, operation: %s\n", getpid(), getppid(), num_fork, operation);



	while(num_fork)
	{
		pid = fork();
		assert(pid >= 0);
		if(pid != 0)
		{
			break;
		}

		//write to sysfs file
		fd_1 = open("/sys/kernel/cs614_sysfs/cs614_value", O_WRONLY);
		assert(fd_1 >= 0);

		memset(operation, '\0', LEN);
		operation[0] = '0' + (getpid() % NUM_OPS);

		ret = write(fd_1, operation, 1);
		assert(ret == 1);


		//pid of the next forked process should not always be equal to the pid of current process + 1
		rand_num = rand()%4;
		for(int i = 0; i < rand_num; i++)
		{
			system("ls > /dev/null");
		}	
		--num_fork;
                printf("pid: %d, ppid: %d, num_fork: %d, operation: %s\n", getpid(), getppid(), num_fork, operation);
	}
	//wait(0);
	//printf("Exiting: pid: %d, ppid: %d\n", getpid(), getppid());
	wait(&wait_status);
	if(WIFEXITED(wait_status))
	{
		printf("exit/return value of child: %d\n", WEXITSTATUS(wait_status));
		if(WEXITSTATUS(wait_status) != 0)
		{
			return -1;
		}
	}
	


	//read from char device
	fd_2 = open("/dev/cs614_device", O_RDONLY);
	assert(fd_2 >= 0);

	memset(buf, '\0', SIZE);
	ret = read(fd_2, &buf, SIZE);
	assert(ret > 0);
	close(fd_1);
	close(fd_2);
	printf("Hello\n");


	getrusage(RUSAGE_SELF, &usage);

        if((getpid()%NUM_OPS == 0) && (atoi(buf) == getpid()))
        {
                printf("1.[pid: %d, ppid: %d]ret: %d, val read: %d\n", getpid(), getppid(), ret, atoi(buf));
		return 0;
        }
        else if((getpid()%NUM_OPS == 1) && (atoi(buf) == 120+getpriority(PRIO_PROCESS, 0)))
        {
                printf("2.[pid: %d, ppid: %d]ret: %d, val read: %d\n", getpid(), getppid(), ret, atoi(buf));
		return 0;
        }
	//Todo: change offset to be added to argv[0]
        else if((getpid()%NUM_OPS == 2) && (!strcmp(argv[0]+2, buf)))
        {
                printf("3.[pid: %d, ppid: %d]ret: %d, val read: %s\n", getpid(), getppid(), ret, buf);
		return 0;
        }
        else if((getpid()%NUM_OPS == 3) && (atoi(buf) == getppid()))
        {
                printf("4.[pid: %d, ppid: %d]ret: %d, val read: %d\n", getpid(), getppid(), ret, atoi(buf));
		return 0;
        }
        else if((getpid()%NUM_OPS == 4) && (atoi(buf) == (usage.ru_nvcsw)))
        {
                printf("5.[pid: %d, ppid: %d]ret: %d, val read: %lu, usage.ru_nvcsw: %lu\n", getpid(), getppid(), ret, atol(buf), usage.ru_nvcsw);
		return 0;
        }
	else
	{
                printf("pid: %d, ppid: %d, operation: %s, val read: %d\n", getpid(), getppid(), operation, atoi(buf));
	}

	return -1;
}
