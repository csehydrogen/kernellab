#include<stdio.h>
#include<sys/ioctl.h>
#include"my_dev.h"

#define EVENT_NUM 0x79
#define USR_FLG (1<<16)
#define OS_FLG (1<<17)
#define INT_FLG (1<<20)
#define ENABLE_FLG (1<<(ENABLE_BIT))

static int file_desc;
static int dump[4190];

int main(){
    //unsigned long select_bit = EVENT_NUM | USR_FLG | ENABLE_FLG | OS_FLG | INT_FLG;
    //unsigned long select_bit = 0x5101b7;
    unsigned long select_bit = 0x0041010E;
    int i;

    file_desc = open(DEVICE_FILE_NAME, 0);

    printf("Before select PMU: %08x\n", ioctl(file_desc, IOCTL_READ_PMU));
    printf("Select PMU register: %08lx\n", select_bit);

    ioctl(file_desc, IOCTL_SELECT_PMU, select_bit);
    printf("delay...\n");
    for(i=0;i<4190;i++) dump[i] = i;

    printf("Read new PMU: %08x\n", ioctl(file_desc, IOCTL_READ_PMU));
    printf("Read TSC: %08x\n", ioctl(file_desc, IOCTL_READ_TSC));
    return 0;
}
