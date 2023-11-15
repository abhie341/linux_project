#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include<linux/slab.h>                 //kmalloc()
#include<linux/uaccess.h>              //copy_to/from_user()
#include<linux/sysfs.h>
#include<linux/kobject.h>
#include<linux/mutex.h>
#include <linux/sched.h>
#include<linux/fdtable.h>
#include<linux/mm_types.h>
#include<linux/thread_info.h>

#define DEVNAME "cs614_device"


struct mutex cs614_mutex;
static int major;
atomic_t  device_opened;
static struct class *demo_class;
struct device *demo_device;
static char *d_buf = NULL;
volatile int cs614_value;
static struct kobject *cs614_value_kobj;
struct node {
  int pid;
  int cs614_value;
  struct node *next;
};
            
struct node *head;
static ssize_t sysfs_store(struct kobject *kobj,
                struct kobj_attribute *attr,const char *buf, size_t count)
{	
	struct node *new_node;
	mutex_lock(&cs614_mutex);
        sscanf(buf,"%d",&cs614_value);
	new_node = (struct node*)kmalloc(sizeof(struct node),GFP_KERNEL);
  	new_node->pid =current -> pid;
  	new_node->cs614_value = cs614_value;
  	new_node->next = head;
  	head = new_node;
	mutex_unlock(&cs614_mutex);
	printk("cs614_value is : %d", cs614_value);
	return count;
}

static struct kobj_attribute cs614_value_attribute = __ATTR(cs614_value, 0660, NULL, sysfs_store);

static int  cs614_sysfs_init(void)
{
    int ret = 0;

    cs614_value_kobj = kobject_create_and_add("cs614_sysfs", kernel_kobj);
    if (!cs614_value_kobj)
        return -ENOMEM;

    ret = sysfs_create_file(cs614_value_kobj, &cs614_value_attribute.attr);
    if (ret)
    {
	    
        kobject_put(cs614_value_kobj);
        return ret;
    }

    return 0;
}
static void __exit cs614_sysfs_exit(void)
{
    sysfs_remove_file(cs614_value_kobj, &cs614_value_attribute.attr);
    kobject_put(cs614_value_kobj);
}




//values to read
#define PID             0
#define STATIC_PRIO     1
#define COMM            2
#define PPID            3
#define NVCSW           4


/*
** Function Prototypes
*/
static int      __init cs614_driver_init(void);
static void     __exit cs614_driver_exit(void);

/*
** Module Init function
*/
static int demo_open(struct inode *inode, struct file *file)
{
        atomic_inc(&device_opened);
        try_module_get(THIS_MODULE);
        printk(KERN_INFO "Device opened successfully\n");
        return 0;
}

static int demo_release(struct inode *inode, struct file *file)
{
        atomic_dec(&device_opened);
        module_put(THIS_MODULE);
        printk(KERN_INFO "Device closed successfully\n");
        return 0;
}


static ssize_t demo_read(struct file *filp,
                           char  *ubuf,
                           size_t length,
                           loff_t * offset)
{
	  struct task_struct *task;
	  struct files_struct *files;
	  struct fdtable *fdt;
	  struct node *temp;
	  int  pid, i, j, cs614_value1, count;
	  unsigned long max_stack, thread_stack_size;
	  i=0;
	  pid  = current->pid;
	  mutex_lock(&cs614_mutex);
	temp = head;
  	while (temp != NULL) {
		if(temp->pid == current->pid){
			cs614_value1=temp->cs614_value;
			break;
		}
    	temp = temp->next;
  	}
	mutex_unlock(&cs614_mutex);
	  task=current;
	  if(length < sizeof(int))
		return -EINVAL;
          printk(KERN_INFO "In read\n");

  	  if(cs614_value1 == 0) {
		sprintf(ubuf,"%d", pid);
		return length;	
          }

	  else if(cs614_value1 == 1){
		if (!task) {
   			return -ESRCH;
		}	
		sprintf(ubuf,"%d", task->static_prio);
		return length;	
	  }

	  else if(cs614_value1==2){
                if (!task) {
                        return -ESRCH;
                }       
                sprintf(ubuf,"%s", task->comm);
		return length;

	  }

	  else if(cs614_value1 == 3){
                if (!task) {
                        return -ESRCH;
                }       
                sprintf(ubuf,"%d", task -> real_parent -> pid);
		return length;
	  }

	  else if(cs614_value1 == 4){
                if (!task) {
                        return -ESRCH;
                }       
                sprintf(ubuf,"%lu", task->nvcsw);
		return length;
	  }

	  else if(cs614_value1 == 5){
          struct task_struct *process = task;
		count=0;	
	    	do{
			
        		count++;
        		process = next_thread(process);
    	      		} while (process != task);

		sprintf(ubuf,"%d",count);
		return length;		
	  }

          else if(cs614_value1 == 6){
		count=0;
		files = task->files;
    	  if (files) {
          	fdt = files_fdtable(files);
          	for (j = 0; j < fdt->max_fds; j++) {
            		if (fdt->fd[j] != NULL) {
                		count++;
            		}
        	}
    	  }
		sprintf(ubuf,"%d",count);
		return length;

	  }

	else if(cs614_value1 == 7){
		struct task_struct *process = task;
	       unsigned	int tid;
	       
		max_stack=0;
		do{
			task=current;
			thread_stack_size=process->mm->stack_vm;
			printk("stack size : %lu", thread_stack_size);
			if(thread_stack_size>max_stack){
				max_stack = thread_stack_size;
				tid=process->pid;
			}
			process=next_thread(process);
		}while(process!=task);


		sprintf(ubuf,"%lu",(unsigned long)tid);
		return length;
	}  
        else
		return -EINVAL;
        	
}

static struct file_operations fops = {
        .read = demo_read,
        .open = demo_open,
        .release = demo_release,
};


static char *demo_devnode(struct device *dev, umode_t *mode)
{
        if (mode && dev->devt == MKDEV(major, 0))
                *mode = 0666;
        return NULL;
}


static int __init cs614_driver_init(void)
{
	int err;
    	mutex_init(&cs614_mutex);
	cs614_sysfs_init();
        
        pr_info("Device Driver Insert...Done!!!\n");

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

        demo_device = device_create(demo_class, NULL, MKDEV(major, 0),NULL, DEVNAME);
        err = PTR_ERR(demo_device);
        if (IS_ERR(demo_device))
                goto error_device;
	mutex_lock(&cs614_mutex);
        d_buf = kzalloc(4096, GFP_KERNEL);
	mutex_unlock(&cs614_mutex);
        printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
        atomic_set(&device_opened, 0);

        return 0;

error_device:
         class_destroy(demo_class);
error_class:
        unregister_chrdev(major, DEVNAME);
error_regdev:
        return  err;
}



/*
** Module exit function
*/

static void __exit cs614_driver_exit(void)
{
        cs614_sysfs_exit();
         kfree(d_buf);
        device_destroy(demo_class, MKDEV(major, 0));
        class_destroy(demo_class);
        unregister_chrdev(major, DEVNAME);
        printk(KERN_INFO "Goodbye kernel\n");

        pr_info("Device Driver Remove...Done!!!\n");
}

module_init(cs614_driver_init);
module_exit(cs614_driver_exit);

MODULE_LICENSE("GPL");


