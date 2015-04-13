#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include "my_dev.h"

static int open_count = 0;

static int device_open(struct inode *inode, struct file *file){
    // only one process can open at the same time
    if(open_count) return -EBUSY;
    open_count++;

    // increment use count
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file){
    // ready for another caller
    open_count--;

    // decrement use count
    module_put(THIS_MODULE);

    return SUCCESS;
}

static void write_msr(struct MsrInOut* msrInOut){
    printk(KERN_INFO "wrmsr: ecx=%x, edx=%x, eax=%x\n", msrInOut->ecx, msrInOut->edx, msrInOut->eax);
    __asm__ __volatile__("wrmsr" :: "c"(msrInOut->ecx), "a"(msrInOut->eax), "d"(msrInOut->edx));
}

static struct MsrInOut read_msr(unsigned int ecx){
    struct MsrInOut ret;
    printk(KERN_INFO "rdmsr: ecx=%x\n", ecx);
    __asm__ __volatile__("rdmsr" : "=a"(ret.eax), "=d"(ret.edx) : "c"(ecx));
    return ret;
}

static struct MsrInOut read_tsc(void){
    struct MsrInOut ret;
    __asm__ __volatile__("rdtsc" : "=a"(ret.eax), "=d"(ret.edx));
    return ret;
}

long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param){
    static size_t buf_len = 0;
    static struct task_struct *task = NULL;
    static struct MsrInOut msrInOut;
    static unsigned long long event_code = 0, ret_val = 0;
    int i;
    
    switch(ioctl_num){
        // Assignment 1
        case IOCTL_SET_BUFFER_LENGTH:
            buf_len = ioctl_param;
            return SUCCESS;

        case IOCTL_GET_PID:
            if(task == NULL)
                return FAIL;
            return task->pid;

        case IOCTL_GET_NAME:
            if(task == NULL)
                return FAIL;
            for(i = 0; task->comm[i] != 0 && i + 1 < buf_len; i++)
                put_user(task->comm[i], (char*)ioctl_param + i);
            put_user('\0', (char*)ioctl_param + i);
            return SUCCESS;

        case IOCTL_MOVE_TO_PARENT:
            if(task == NULL || (task = task->parent) == NULL){
                return FAIL;
            }
            return SUCCESS;

        case IOCTL_MOVE_TO_CURRENT:
            task = current;
            return SUCCESS;

        // Assignment 2
        case IOCTL_SELECT_PMU:
            msrInOut.ecx = PERFEVTSEL; // target
            msrInOut.edx = 0;
            msrInOut.eax = 0;
            write_msr(&msrInOut);

            msrInOut.eax = ioctl_param; // value
            write_msr(&msrInOut);

            msrInOut.ecx = PERFCTR;
            msrInOut.edx = 0;
            msrInOut.eax = 0;
            write_msr(&msrInOut);

            return SUCCESS;

        case IOCTL_READ_PMU:
            // fetch current event code
            msrInOut = read_msr(PERFEVTSEL);
            event_code = msrInOut.eax;
            
            // stop counter
            msrInOut.ecx = PERFEVTSEL;
            msrInOut.edx = 0;
            msrInOut.eax = 0;
            write_msr(&msrInOut);

            // read counter
            msrInOut = read_msr(PERFCTR);
            ret_val = msrInOut.eax;

            // restart counter w/ recent event code
            msrInOut.ecx = PERFEVTSEL;
            msrInOut.edx = 0;
            msrInOut.eax = event_code;
            write_msr(&msrInOut);
            return ret_val;

        case IOCTL_READ_TSC:
            msrInOut = read_tsc();
            return msrInOut.value;
    }
    return SUCCESS; // never reach here
}

static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
    .open = device_open,
    .release = device_release
};

int init_module(void){
    int ret;
    ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);

    if(ret < 0){
        printk(KERN_ALERT "Registering char device failed with %d\n", ret);
        return ret;
    }

    return SUCCESS;
}

void cleanup_module(void){
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
}
