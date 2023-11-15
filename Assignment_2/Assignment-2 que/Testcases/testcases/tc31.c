/**
 * CS614: Linux Kernel Programming
 * TC:VMA with only 1 physical at end, compaction should bring it to front
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/fcntl.h>
#include<signal.h>
#include<sys/ioctl.h>
#include<sys/mman.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#define MAJOR_NUM 100
#define SIZE (1<<22)
#define NUM_PAGES 1024

#define IOCTL_MVE_VMA_TO _IOR(MAJOR_NUM, 0, char*)
#define IOCTL_MVE_VMA _IOWR(MAJOR_NUM, 1, char*)
#define IOCTL_PROMOTE_VMA _IOR(MAJOR_NUM, 2, char*)
#define IOCTL_COMPACT_VMA _IOWR(MAJOR_NUM, 3, char*)

struct address{
    unsigned long old;
    unsigned long new;
};

struct input{
    unsigned long addr;
    unsigned length;
    struct address* buff;
};

int main()
{
   char *ptr;
   struct input ip;
   struct address mapping[NUM_PAGES];
   unsigned long new_addr;

   int fd = open("/dev/cs614",O_RDWR);
   if(fd < 0){
       perror("open");
       exit(-1);
   }

   ptr = (char*)mmap(NULL, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
   unsigned long page_align = ((unsigned long)ptr+4096)&~(0xfff);
   munmap(ptr,SIZE);
   ptr = (char*)mmap((void*)page_align, SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
   
   if(ptr == MAP_FAILED){
        perror("mmap");
        exit(-1);
   }
   unsigned index = SIZE-4096;
   ptr[index] = 'A';

   ip.addr = (unsigned long)ptr;
   ip.length = (SIZE>>12);
   ip.buff = mapping;
   
   printf("going to compact VMA ptr [%lx], length:%u, mapping:%lx\n",ip.addr,ip.length,(unsigned long)ip.buff);

   if(ioctl(fd, IOCTL_COMPACT_VMA, &ip) < 0){
       printf("Testcase failed\n");
       exit(-1);
   }
   struct address * temp;
   unsigned page_index = (&ptr[index]-ptr)>>12;
   unsigned page_offset = ((unsigned long)&ptr[index])&0xfff;
   printf("index into mapping:%u,offset:%u\n",page_index,page_offset);
   temp = (struct address*)ip.buff+page_index;
   new_addr = (temp->new)<<12+page_offset;
   printf("old address:%lx, new address:%lx\n",(unsigned long)&ptr[index],new_addr);
   if(temp->new != (unsigned long)ptr>>12){
       printf("Testcase Failed\n");
       return -1;
   }
   close(fd);
   return 0;
}
