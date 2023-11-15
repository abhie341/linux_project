#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/mm.h>
#include<linux/mm_types.h>
#include<linux/file.h>
#include<linux/fs.h>
#include<linux/path.h>
#include<linux/slab.h>
#include<linux/dcache.h>
#include<linux/sched.h>
#include<linux/uaccess.h>
#include<linux/fs_struct.h>
#include <asm/tlbflush.h>
#include<linux/uaccess.h>
#include<linux/device.h>
#include <linux/kthread.h> 
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include "btplus.h"

static int major;
atomic_t  device_opened;
static struct class *demo_class;
struct device *demo_device;
struct kobject *cs614_kobject;
unsigned promote = 0;

static ssize_t  sysfs_show(struct kobject *kobj,
                        struct kobj_attribute *attr, char *buf);
static ssize_t  sysfs_store(struct kobject *kobj,
                        struct kobj_attribute *attr,const char *buf, size_t count);

struct kobj_attribute sysfs_attr; 

struct address{
    unsigned long from_addr;
    unsigned long to_addr;
};

struct input{
    unsigned long addr;
    unsigned length;
    struct address * buff;
};

static int device_open(struct inode *inode, struct file *file)
{
        atomic_inc(&device_opened);
        try_module_get(THIS_MODULE);
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        printk(KERN_INFO "Device closed successfully\n");

        return 0;
}


static ssize_t device_read(struct file *filp,
                           char *buffer,
                           size_t length,
                           loff_t * offset){
    printk("read called\n");
    return 0;
}

static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off){
    
    printk("write called\n");
    return 8;
}


long device_ioctl(struct file *file,	
		 unsigned int ioctl_num,
		 unsigned long ioctl_param)
{
	unsigned long prot =0,flags = 0;
	//unsigned long addr = 1234;
	int ret = 0; // on failure return -1
	struct address * buff = NULL;
	unsigned long vma_addr = 0;
	unsigned long to_addr = 0;
	unsigned long from_addr =0;
	unsigned length = 0;
	struct input* ip;
	unsigned index = 0;
	struct address temp;
	unsigned long ptr1, j=0;
	struct vm_area_struct *vma;
	struct mm_struct *mm;
	struct page *page;
    	unsigned long size = 0;
	char *buff1 = (char*) kmalloc(4096, GFP_KERNEL);
	int iterations=0;
	mm=current->mm;
	/*
	 * Switch according to the ioctl called
	 */
	switch (ioctl_num) {
	case IOCTL_MVE_VMA_TO:
	    buff = (struct address*)vmalloc(sizeof(struct address)) ;
	    printk("move VMA at a given address");
	    if(copy_from_user(buff,(char*)ioctl_param,sizeof(struct address))){
	        pr_err("MVE_VMA address write error\n");
		return ret;
	    }
	    vma_addr = buff->from_addr;
	    to_addr = buff->to_addr;
	    if(!mtree_load(&mm->mm_mt,vma_addr))
                      return -1;
	    if(mtree_load(&mm->mm_mt,to_addr))
                      return -1;
	    
	    vma = find_vma(current->mm, vma_addr);
	    if (!vma || vma->vm_start > vma_addr){
		printk("Given old address is not in any VMA");
            	return -1;
	    }
	    if(!vma->vm_ops)
        	 flags|=MAP_ANONYMOUS;
            if(vma->vm_flags & VM_READ)
         	prot|=PROT_READ;
            if(vma->vm_flags & VM_WRITE)
         	prot|=PROT_WRITE;
            if(vma->vm_flags & VM_EXEC)
         	prot|=PROT_EXEC;
            if(vma->vm_flags & VM_SHARED)
         	flags|=MAP_SHARED;
            else
         	flags|=MAP_PRIVATE;
    	    size = vma_pages(vma) * PAGE_SIZE;
            size -= vma_addr - vma->vm_start;
	    printk("KERN_INFO Size of old VMA is %ld",size);
            
	    ptr1=vm_mmap(NULL, to_addr, size, prot, flags, 0);
	    if (IS_ERR((void *)ptr1)){
		printk("error in creating new memory region");
            	return -1;
	    }
	    iterations=size/4096;
	    while(iterations){  
	    if(copy_from_user(buff1,(char*)vma_addr, 4096))
		    return ret;
	    
	    
	    if(copy_to_user((char*)to_addr,buff1, 4096))
                    return ret;
	    vma_addr+=4096;
	    to_addr+=4096;
	    iterations-=1;
	    }
	    vma_addr=buff->from_addr;
	    vm_munmap(vma_addr, size);
	    to_addr=buff->to_addr;

	    printk("address from :%lx, to:%lx \n",vma_addr,to_addr);
	    vfree(buff);
	    return ret;
	case IOCTL_MVE_VMA:
	   // unsigned long j=0;
	   
	    buff = (struct address*)vmalloc(sizeof(struct address)) ;
	    printk("move VMA to available hole address");
	    if(copy_from_user(buff,(char*)ioctl_param,sizeof(struct address))){
	        pr_err("MVE_VMA address write error\n");
		return ret;
	    }
	    vma_addr = buff->from_addr;
	    if(!mtree_load(&mm->mm_mt,vma_addr))
                      return -1;
	    vma = find_vma(current->mm, vma_addr);
	     if (!vma || vma->vm_start > vma_addr){
                printk("Given old address is not in any VMA");
                return -1;
            }
            size = vma_pages(vma) * PAGE_SIZE;
            size -= vma_addr - vma->vm_start;
            printk("KERN_INFO Size of old VMA is %ld",size);
	    printk("VMA address :%lx \n",vma_addr);
	    for(to_addr=vma->vm_end;to_addr<=0x7fffffffffff;to_addr+=4096){
		
			if(!mtree_load(&mm->mm_mt,to_addr)){
				j+=1;
				if(j==size/4096+1)
					break;		
			}
			else
				j=0;
	    }
	    if(j!=size/4096+1){
	    	printk("There is no hole with the given space");
		return -1;
	    }

	    to_addr-=size;
	    buff->to_addr=to_addr;
	    if(mtree_load(&mm->mm_mt,to_addr))
            	return -1;
    	    printk(KERN_INFO "find_hole: Hole found at address 0x%lx\n", to_addr);
	    if(!vma->vm_ops)
         	flags|=MAP_ANONYMOUS;
            if(vma->vm_flags & VM_READ)
         	prot|=PROT_READ;
            if(vma->vm_flags & VM_WRITE)
         	prot|=PROT_WRITE;
            if(vma->vm_flags & VM_EXEC)
         	prot|=PROT_EXEC;
            if(vma->vm_flags & VM_SHARED)
         	flags|=MAP_SHARED;
            else
         	flags|=MAP_PRIVATE;
	    ptr1=vm_mmap(NULL, to_addr, size, prot, flags, 0);
            if (IS_ERR((void *)ptr1)){
                printk("error in creating new memory region");
                return -1;
            }
            iterations=size/4096;
            while(iterations){
            if(copy_from_user(buff1,(char*)vma_addr, 4096))
                    return ret;

            if(copy_to_user((char*)to_addr,buff1, 4096))
                    return ret;
            vma_addr+=4096;
            to_addr+=4096;
            iterations-=1;
            }
            vma_addr=buff->from_addr;
            vm_munmap(vma_addr, size);
	    to_addr=buff->to_addr;
            printk("address from :%lx, to:%lx \n",vma_addr,to_addr);
	    if(copy_to_user((char*)ioctl_param,buff,sizeof(struct address))){
	        pr_err("MVE_VMA address error\n");
		return -1;
	    }
	    vfree(buff);
	    return ret;
        case IOCTL_PROMOTE_VMA:
	    unsigned long i=0,start_addr=0;
	    unsigned long pfn;
	    unsigned long virt_addr=0;
            pte_t *pte, entry;
	    pgd_t *pgd;
            p4d_t *p4d;
            pud_t *pud;
            pmd_t *pmd;
            printk("promote 4KB pages to 2\n");
            for(i=0;i<=0x7fffffffffff;i+=4096){
            	if(mtree_load(&mm->mm_mt,i)){
                	j+=1;
                        if(j==512){
				start_addr=i;
				start_addr-=2*1024*1024;
				page = alloc_pages(GFP_KERNEL,9);
				pfn = page_to_pfn(page);
				virt_addr =(unsigned long) page_address(page);	
				memcpy((void *)virt_addr, (void *)start_addr,2*1024*1024);
				for(int k=1;k<=512;k+=1){
					pgd = pgd_offset(current->mm, virt_addr);
                                        p4d = p4d_offset(pgd, virt_addr);
                                        pud = pud_offset(p4d, virt_addr);
                                        pmd = pmd_offset(pud, virt_addr);
                                        pte = pte_offset_kernel(pmd, virt_addr);
					entry=*pte;
				
					set_pte(pte, __pte(0));
					pgd = pgd_offset(current->mm, start_addr);
					p4d = p4d_offset(pgd, start_addr);
					pud = pud_offset(p4d, start_addr);
					pmd = pmd_offset(pud, start_addr);	
					pte = pte_offset_kernel(pmd, start_addr);
					*pte=entry;
					virt_addr+=4096;
					start_addr+=4096;
					__flush_tlb_local();
				}
				start_addr-=2*1024*1024;
				virt_addr-=2*1024*1024;
				vm_munmap(virt_addr,2*1024*1024);
				vma = find_vma(current->mm, start_addr);
				if (remap_pfn_range(vma, vma->vm_start, pfn, 2*1024*1024, vma->vm_page_prot)) {
        				printk(KERN_ERR "Failed to remap physical memory\n");
        				return -EAGAIN;
					
    				}
				j=0;
			}
                 }
                 else
                 	j=0;
            }

            
	     
	    return ret;
	case IOCTL_COMPACT_VMA:
	    i=0,j=0;
	    printk("compact VMA\n");
	    ip = (struct input*)vmalloc(sizeof(struct input)) ;
	    if(copy_from_user(ip,(char*)ioctl_param,sizeof(struct input))){
                pr_err("MVE_MERG_VMA address write error\n");
                return ret;
            }
	    vma_addr = ip->addr;
	    length = ip->length;
	    buff = ip->buff;
	    temp.from_addr = vma_addr;
	    temp.to_addr = vma_addr;
	    printk("vma address:%lx, length:%u, buff:%lx\n",vma_addr,length,(unsigned long)buff);
	    //populate old to new address mapping in user buffer.
	    //number of entries in this buffer is equal to the number of 
	    //virtual pages in vma address range
	    //index of moved addr in mapping table is , index = (addr-vma_address)>>12
	    
	    from_addr=vma_addr+length*4096;
	    to_addr=vma_addr;
	    j=from_addr;
	    i=to_addr;
     	    vma = find_vma(current->mm, from_addr);

	    while(i<j){
	    	for(i=to_addr;i<j;i+=4096){
	    		if(!mtree_load(&mm->mm_mt,i)){
				to_addr=i;
				break;
			}
			else{
				temp.from_addr=to_addr;
				temp.to_addr=to_addr;
				index = (to_addr-vma_addr)>>12;
            			if(copy_to_user((struct address *)buff+index, &temp, sizeof(struct address))){
                			pr_err("COMPACT VMA read error\n");
                			return ret;
            			}

			}

	    	} 
	    	for(j=from_addr;i<j;j-=4096){
	    		if(mtree_load(&mm->mm_mt,j)){
				from_addr=j;
				break;
			}
	    	}
		if(j>=i)
			break;
		if(copy_from_user(buff1,(char*)to_addr, 4096))
                    return ret;
            	if(copy_to_user((char*)from_addr,buff1, 4096))
                    return ret;
		if (remap_pfn_range(vma, from_addr, 0, 4096, PAGE_KERNEL)) {
 	   		printk(KERN_ERR "Failed to remap virtual memory range\n");
    			return -EINVAL;
		}
		temp.from_addr=from_addr;
                temp.to_addr=to_addr;
                index = (from_addr-vma_addr)>>12;
                                if(copy_to_user((struct address *)buff+index, &temp, sizeof(struct address))){
                                        pr_err("COMPACT VMA read error\n");
                                        return ret;
                                }
		temp.from_addr=to_addr;
                temp.to_addr=from_addr;
                index = (to_addr-vma_addr)>>12;
                                if(copy_to_user((struct address *)buff+index, &temp, sizeof(struct address))){
                                        pr_err("COMPACT VMA read error\n");
                                        return ret;
                                }		

	    }
	    index = (vma_addr-vma_addr)>>12;
	    if(copy_to_user((struct address *)buff+index, &temp, sizeof(struct address))){
	        pr_err("COMPACT VMA read error\n");
		return ret;
	    } 
	    vfree(ip);
            return ret;
	}
	return ret;
}


static struct file_operations fops = {
        .read = device_read,
        .write = device_write,
	.unlocked_ioctl = device_ioctl,
        .open = device_open,
        .release = device_release,
};

static char *demo_devnode(struct device *dev, umode_t *mode)
{
        if (mode && dev->devt == MKDEV(major, 0))
                *mode = 0666;
        return NULL;
}

//Implement required logic
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{
    pr_info("sysfs read\n"); 
    return 0;
}

//Implement required logic
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr,
                     const char *buf, size_t count)
{
    printk("sysfs write\n");
    return count;
}



int init_module(void)
{
        int err;
	printk(KERN_INFO "Hello kernel\n");
        major = register_chrdev(0, DEVNAME, &fops);
        err = major;
        if (err < 0) {      
             printk(KERN_ALERT "Registering char device failed with %d\n", major);   
             goto error_regdev;
        }                 
        
        demo_class = class_create(THIS_MODULE, DEVNAME);
        err = PTR_ERR(demo_class);
        if (IS_ERR(demo_class))
                goto error_class;

        demo_class->devnode = demo_devnode;

        demo_device = device_create(demo_class, NULL,
                                        MKDEV(major, 0),
                                        NULL, DEVNAME);
        err = PTR_ERR(demo_device);
        if (IS_ERR(demo_device))
                goto error_device;
 
        printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);                                                              
        atomic_set(&device_opened, 0);
        
	cs614_kobject = kobject_create_and_add("kobject_cs614", kernel_kobj);
        
	if(!cs614_kobject)
            return -ENOMEM;
	
	sysfs_attr.attr.name = "promote";
	sysfs_attr.attr.mode = 0666;
	sysfs_attr.show = sysfs_show;
	sysfs_attr.store = sysfs_store;

	err = sysfs_create_file(cs614_kobject, &(sysfs_attr.attr));
	if (err){
	    pr_info("sysfs exists:");
	    goto r_sysfs;
	}
	return 0;
r_sysfs:
	kobject_put(cs614_kobject);
        sysfs_remove_file(kernel_kobj, &sysfs_attr.attr);
error_device:
         class_destroy(demo_class);
error_class:
        unregister_chrdev(major, DEVNAME);
error_regdev:
        return  err;
}

void cleanup_module(void)
{
        device_destroy(demo_class, MKDEV(major, 0));
        class_destroy(demo_class);
        unregister_chrdev(major, DEVNAME);
	kobject_put(cs614_kobject);
	sysfs_remove_file(kernel_kobj, &sysfs_attr.attr);
	printk(KERN_INFO "Goodbye kernel\n");
}

MODULE_AUTHOR("cs614");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("assignment2");
