#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/resource.h>

#define SIZE 16 

int main(int argc, char* argv[])
{
	int fd_1 = 0;
	int fd_2 = 0;
	char buf[SIZE];
	int val = 0;
	int ret = 0;
 	struct rusage usage;

	if(argc != 1)
	{
		printf("Format: $./executable\n");
		return -1;
	}


	//write to sysfs file
	fd_1 = open("/sys/kernel/cs614_sysfs/cs614_value", O_WRONLY);
	assert(fd_1 >= 0);

	ret = write(fd_1, "4", 1);
	printf("write(): ret: %d\n", ret);
	assert(ret == 1);

	
	//read from char device
	fd_2 = open("/dev/cs614_device", O_RDONLY);
	assert(fd_2 >= 0);

	memset(buf, '\0', SIZE);
	sleep(1);
        sleep(1);
        sleep(1);
	ret = read(fd_2, &buf, SIZE);
	assert(ret > 0);
	close(fd_1);
	close(fd_2);

	printf("read(): ret: %d, val read: %d, getpid: %d\n", ret, atoi(buf), getpid());
	getrusage(RUSAGE_SELF, &usage);
	if((atoi(buf) == (usage.ru_nvcsw)))
        {
                printf("ret: %d, val read: %lu, usage.ru_nvcsw: %lu\n", ret, atol(buf), usage.ru_nvcsw);
                printf("Testcase passed\n");
                return 0;
        }
	printf("Testcase failed\n");
	
	return -1;
}
