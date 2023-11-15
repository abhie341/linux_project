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

#define SIZE 16 

int main(int argc, char* argv[])
{
	int fd_1 = 0;
	int fd_2 = 0;
	char buf[SIZE];
	int val = 0;
	int ret = 0;
	int pid = 0;
        struct rusage usage;

	if(argc != 1)
	{
		printf("Format: $./executable\n");
		return -1;
	}


	//write to sysfs file
	fd_1 = open("/sys/kernel/cs614_sysfs/cs614_value", O_WRONLY);
	assert(fd_1 >= 0);

	ret = write(fd_1, "0", 1);
	assert(ret == 1);


	ret = write(fd_1, "1", 1);
	assert(ret == 1);

	ret = write(fd_1, "3", 1);
	assert(ret == 1);

	//read from char device
	fd_2 = open("/dev/cs614_device", O_RDONLY);
	assert(fd_2 >= 0);



	pid = fork();
	assert(pid >= 0);
	if(pid == 0)
	{
		printf("Inside Child\n");
		ret = write(fd_1, "1", 1);
		assert(ret == 1);
		ret = read(fd_2, &buf, SIZE);
		assert(ret > 0);
		close(fd_1);
		close(fd_2);

		if(atoi(buf) == 120+getpriority(PRIO_PROCESS, 0))
		{
			printf("Child: ret: %d, val read: %d\n", ret, atoi(buf));
			return 0;
		}
		printf("Child: Testcase failed\n");
		return -1;
	}
	else
	{
		printf("Parent: waiting for child\n");
		wait(&ret);
		printf("Parent: wait ended\n");
		assert(ret == 0);

		memset(buf, '\0', SIZE);
		ret = read(fd_2, &buf, SIZE);
		assert(ret > 0);

		printf("Parent: ret: %d, val read: %d\n", ret, atoi(buf));
		if((atoi(buf) != getppid()))
		{
			printf("Parent: Testcase failed\n");
			return -1;
		}

		memset(buf, '\0', SIZE);
		ret = read(fd_2, &buf, SIZE);
		assert(ret > 0);

		printf("Parent: ret: %d, val read: %d\n", ret, atoi(buf));
		if((atoi(buf) != getppid()))
		{
			printf("Parent: Testcase failed\n");
			return -1;
		}

		close(fd_1);
		close(fd_2);

		printf("Parent: Testcase passed\n");
		return 0;
	}
	return 0;
}
