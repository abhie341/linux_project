#include <crypter.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define SET_KEY  _IOW(243, 1, struct cc_key)
#define SET_CONFIG  _IOW(243, 2, struct cfg)
#define ENCRYPT  _IOW(243, 3, struct operations *)
#define DECRYPT  _IOW(243, 4, struct operations *)

static uint64_t map_size;

struct cc_key {
    uint8_t key_a;
    uint8_t key_b;
};

struct cfg {
    uint8_t mode;
    uint8_t set;
};

struct operations {
	void * addr;
	uint64_t length;
	uint8_t isMapped;
};
/*Function template to create handle for the CryptoCard device.
On success it returns the device handle as an integer*/
DEV_HANDLE create_handle()
{
  int fd;

	fd = open("/dev/cryptocard",O_RDWR);
	if (fd < 0) {
		perror("open");
		return ERROR;
	}  

	return fd;
}

/*Function template to close device handle.
Takes an already opened device handle as an arguments*/
void close_handle(DEV_HANDLE cdev)
{
	close(cdev);
}

/*Function template to encrypt a message using MMIO/DMA/Memory-mapped.
Takes four arguments
  cdev: opened device handle
  addr: data address on which encryption has to be performed
  length: size of data to be encrypt
  isMapped: TRUE if addr is memory-mapped address otherwise FALSE
*/
int encrypt(DEV_HANDLE cdev, ADDR_PTR addr, uint64_t length, uint8_t isMapped)
{
  struct operations op;
    int err;


    	op.addr = addr;
    	op.length = length;
		op.isMapped = isMapped;
/*
    	err = ioctl(cdev,ENCRYPT, &op);
    	if (err == -1)
        	return ERROR;
	
	    return err;
	
		int count, temp, chunk = 0xfffff - 0xa8;
		char *new = (char *)addr;

		count = length/chunk;
		temp =  length%chunk;
		
		op.isMapped = isMapped;
		for(int i=0;i<count;i++){
    			op.addr = new;
    			op.length = chunk ;
			err = ioctl(cdev,ENCRYPT, &op);
	        	if (err == -1)
    	        		return ERROR;
			new = new + chunk;
		}
		if(temp != 0){
            		op.addr = new;
            		op.length = temp ;
  */          		err = ioctl(cdev,ENCRYPT, &op);
            		if (err == -1)
                		return ERROR;

	//	}
        	return err;
							
	
}

/*Function template to decrypt a message using MMIO/DMA/Memory-mapped.
Takes four arguments
  cdev: opened device handle
  addr: data address on which decryption has to be performed
  length: size of data to be decrypt
  isMapped: TRUE if addr is memory-mapped address otherwise FALSE
*/
int decrypt(DEV_HANDLE cdev, ADDR_PTR addr, uint64_t length, uint8_t isMapped)
{
  struct operations op;
    int err;
   
	
    	op.addr = addr;
    	op.length = length;
	op.isMapped = isMapped;
   /* 
   	err = ioctl(cdev,DECRYPT, &op);
    	if (err == -1)
        	return ERROR;
    
    	return err;
	
        int count, temp, chunk = 0xfffff - 0xa8;
        char *new = (char *)addr;

        count = length/chunk;
        temp =  length%chunk;

        op.isMapped = isMapped;
        for(int i=0;i<count;i++){
            op.addr = new;
            op.length = chunk ;
            err = ioctl(cdev,DECRYPT, &op);
            if (err == -1)
                return ERROR;
            new = new + chunk;
        }*/
       // if(temp != 0){

       //     op.addr = new;
         //   op.length = temp;
            err = ioctl(cdev,DECRYPT, &op);
            if (err == -1)
                return ERROR;

       // }
        return err;
    

}

/*Function template to set the key pair.
Takes three arguments
  cdev: opened device handle
  a: value of key component a
  b: value of key component b
Return 0 in case of key is set successfully*/
int set_key(DEV_HANDLE cdev, KEY_COMP a, KEY_COMP b)
{
  struct cc_key key;
	int err;	

	key.key_a = a;
	key.key_b = b;

	err = ioctl(cdev,SET_KEY, &key);
	if (err == -1)
  		return ERROR;
	
	return err;
}

/*Function template to set configuration of the device to operate.
Takes three arguments
  cdev: opened device handle
  type: type of configuration, i.e. set/unset DMA operation, interrupt
  value: SET/UNSET to enable or disable configuration as described in type
Return 0 in case of key is set successfully*/
int set_config(DEV_HANDLE cdev, config_t type, uint8_t value)
{
  struct cfg cfg;
    int err;

    cfg.mode = type;
    cfg.set = value;
    err = ioctl(cdev,SET_CONFIG, &cfg);
    if (err == -1)
        return ERROR;

    return err;
}

/*Function template to device input/output memory into user space.
Takes three arguments
  cdev: opened device handle
  size: amount of memory-mapped into user-space (not more than 1MB strict check)
Return virtual address of the mapped memory*/
ADDR_PTR map_card(DEV_HANDLE cdev, uint64_t size)
{
  if(size <= (0x100000-0xa8)) {
  	ADDR_PTR addr = mmap(NULL, size+0xa8, PROT_READ|PROT_WRITE, MAP_SHARED, cdev, 0);
	printf("inside map_card function\n");	
	if(addr == MAP_FAILED) {
		printf("mmap failed in map_card function \n");	
		return NULL;
	}
	
	map_size = size;
	addr = ((uint8_t *)addr) + 0xa8;		
	return addr;
}
else
	return NULL;
}

/*Function template to device input/output memory into user space.
Takes three arguments
  cdev: opened device handle
  addr: memory-mapped address to unmap from user-space*/
void unmap_card(DEV_HANDLE cdev, ADDR_PTR addr)
{
	addr = ((uint8_t*)addr)-0xa8;
	munmap(addr, map_size);
}
