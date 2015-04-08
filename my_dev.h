#ifndef MYDEV_H
#define MYDEV_H

#include <linux/ioctl.h>

#define MAJOR_NUM 1522
#define SUCCESS 0
#define FAIL -1

#define IOCTL_SET_BUFFER_LENGTH _IOR(MAJOR_NUM, 0, char *)
#define IOCTL_GET_PID           _IOR(MAJOR_NUM, 1, char *)
#define IOCTL_GET_NAME          _IOR(MAJOR_NUM, 2, char *)
#define IOCTL_MOVE_TO_PARENT    _IOR(MAJOR_NUM, 3, char *)
#define IOCTL_MOVE_TO_CURRENT   _IOR(MAJOR_NUM, 4, char *)

#define DEVICE_NAME "my_dev"
#define DEVICE_FILE_NAME "/dev/my_dev"

#endif
