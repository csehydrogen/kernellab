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

static void set_msr(struct MsrInOut* msrInOut, unsigned int ecx, unsigned long long value){
    msrInOut->ecx = ecx;
    msrInOut->value = value;
}

static void write_msr(struct MsrInOut* msrInOut){
    printk(KERN_INFO "wrmsr : ecx=%x, edx=%x, eax=%x\n", msrInOut->ecx, msrInOut->edx, msrInOut->eax);
    __asm__ __volatile__("wrmsr" :: "c"(msrInOut->ecx), "a"(msrInOut->eax), "d"(msrInOut->edx));
}

static struct MsrInOut read_msr(unsigned int ecx){
    struct MsrInOut ret;
    ret.ecx = ecx;
    printk(KERN_INFO "rdmsr before : ecx=%x\n", ret.ecx);
    __asm__ __volatile__("rdmsr" : "=a"(ret.eax), "=d"(ret.edx) : "c"(ret.ecx));
    printk(KERN_INFO "rdmsr after  : edx=%x, eax=%x\n", ret.edx, ret.eax);
    return ret;
}

static struct MsrInOut read_tsc(void){
    struct MsrInOut ret;
    __asm__ __volatile__("rdtsc" : "=a"(ret.eax), "=d"(ret.edx));
    printk(KERN_INFO "rdtsc : edx=%x, eax=%x\n", ret.edx, ret.eax);
    return ret;
}

long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param){
    // length of user's buffer
    // set by IOCTL_SET_BUFFER_LENGTH
    static size_t buf_len = 0;
    // task pointed currently
    // manipulated by IOCTL_MOVE_TO_[CURRENT | PARENT]
    static struct task_struct *task = NULL;
    static struct MsrInOut msrInOut;
    static unsigned long long event_code;
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
            // copy task->comm to user's buffer
            for(i = 0; task->comm[i] != 0 && i + 1 < buf_len; i++)
                put_user(task->comm[i], (char*)ioctl_param + i);
            // append null
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
        case IOCTL_RESET_PMU_CTR:
            // set counter to 0
            set_msr(&msrInOut, PERFCTR, 0);
            write_msr(&msrInOut);
            return SUCCESS;

        case IOCTL_SELECT_PMU_EVT:
            // program performance select register
            // and stop counter by setting enable bit to 0
            set_msr(&msrInOut, PERFEVTSEL, ioctl_param & ~ENABLE_FLG);
            write_msr(&msrInOut);
            return SUCCESS;

        case IOCTL_START_PMU_CTR:
            // start counter by setting enable bit to 1
            msrInOut = read_msr(PERFEVTSEL);
            msrInOut.value |= ENABLE_FLG;
            write_msr(&msrInOut);
            return SUCCESS;

        case IOCTL_STOP_PMU_CTR:
            // stop counter by setting enable bit to 0
            msrInOut = read_msr(PERFEVTSEL);
            msrInOut.value &= ~ENABLE_FLG;
            write_msr(&msrInOut);
            return SUCCESS;

        case IOCTL_READ_PMU_CTR:
            // stop counter first
            msrInOut = read_msr(PERFEVTSEL);
            event_code = msrInOut.value;
            msrInOut.value &= ~ENABLE_FLG;
            write_msr(&msrInOut);
            
            // read counter
            msrInOut = read_msr(PERFCTR);
            *(unsigned long long *)ioctl_param = msrInOut.value;

            // restore event select register
            set_msr(&msrInOut, PERFEVTSEL, event_code);
            write_msr(&msrInOut);
            return SUCCESS;

        case IOCTL_READ_TSC:
            msrInOut = read_tsc();
            *(unsigned long long *)ioctl_param = msrInOut.value;
            return SUCCESS;
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
