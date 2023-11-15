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

	ret = write(fd_1, "2", 1);
	printf("write(): ret: %d\n", ret);
	assert(ret == 1);

	
	//read from char device
	fd_2 = open("/dev/cs614_device", O_RDONLY);
	assert(fd_2 >= 0);

	memset(buf, '\0', SIZE);
	ret = read(fd_2, &buf, SIZE);
	assert(ret > 0);
	close(fd_1);
	close(fd_2);

	//Note: Set the offset to be added to argv[0] appropriately
	//During testing, we used absolute path to run current process
	//Name of this process was at offset 50 onwards 
	printf("read(): ret: %d, val read: %s, getpid: %d, argv[0]+2: %s\n", ret, buf, getpid(), argv[0]+50);
	if(!strcmp(argv[0]+50, buf))
        {
                printf("ret: %d, val read: %s\n", ret, buf);
                printf("Testcase passed\n");
                return 0;
        }
	printf("Testcase failed\n");
	
	return -1;
}
