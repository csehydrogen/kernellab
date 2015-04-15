#ifndef MYDEV_H
#define MYDEV_H

#include <linux/ioctl.h>

#define MAJOR_NUM 1522
#define SUCCESS 0
#define FAIL -1

#define DEVICE_NAME "my_dev"
#define DEVICE_FILE_NAME "/dev/my_dev"

// Assignment 1
#define IOCTL_SET_BUFFER_LENGTH _IOR(MAJOR_NUM, 0, size_t)
#define IOCTL_GET_PID           _IOR(MAJOR_NUM, 1, void *)
#define IOCTL_GET_NAME          _IOR(MAJOR_NUM, 2, char *)
#define IOCTL_MOVE_TO_PARENT    _IOR(MAJOR_NUM, 3, void *)
#define IOCTL_MOVE_TO_CURRENT   _IOR(MAJOR_NUM, 4, void *)

// Assignment 2
#define IOCTL_RESET_PMU_CTR     _IOR(MAJOR_NUM, 5, void *)
#define IOCTL_SELECT_PMU_EVT    _IOR(MAJOR_NUM, 6, unsigned int)
#define IOCTL_START_PMU_CTR     _IOR(MAJOR_NUM, 7, void *)
#define IOCTL_STOP_PMU_CTR      _IOR(MAJOR_NUM, 8, void *)
#define IOCTL_READ_PMU_CTR      _IOR(MAJOR_NUM, 9, unsigned long long *)
#define IOCTL_READ_TSC          _IOR(MAJOR_NUM, 10, unsigned long long *)

#define PERFEVTSEL  0x186
#define PERFCTR     0xc1

#define USR_FLG     (1<<16)
#define OS_FLG      (1<<17)
#define INT_FLG     (1<<20)
#define ENABLE_FLG  (1<<22)

#define EVT_UOPS_ISSUED 0x010E

struct MsrInOut {
    unsigned int ecx;             // msr identifier
    union {
        struct {
            unsigned int eax;     // low double word
            unsigned int edx;     // high double word
        };
        unsigned long long value; // quad word
    };
};

#endif
