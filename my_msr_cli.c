#include <stdio.h>
#include <sys/ioctl.h>
#include "my_dev.h"

static int file_desc;
static int dump[4190];

void read_pmu_ctr(){
    unsigned long long tmp;
    ioctl(file_desc, IOCTL_READ_PMU_CTR, &tmp);
    printf("Current counter: %016llx\n", tmp);
}

void read_tsc(){
    unsigned long long tmp;
    ioctl(file_desc, IOCTL_READ_TSC, &tmp);
    printf("Current tsc: %016llx\n", tmp);
}

int main(){
    unsigned long select_bit = EVT_UOPS_ISSUED | USR_FLG;
    int i;
    unsigned long long tmp;

    file_desc = open(DEVICE_FILE_NAME, 0);

    read_pmu_ctr();

    printf("Reset counter...");
    ioctl(file_desc, IOCTL_RESET_PMU_CTR);
    printf("Done\n"); 

    read_pmu_ctr();

    printf("Select event %08lx...", select_bit); 
    ioctl(file_desc, IOCTL_SELECT_PMU_EVT, select_bit);
    printf("Done\n"); 

    read_pmu_ctr();

    printf("Delay...\n");
    for(i=0;i<4190;i++) dump[i] = i;

    read_pmu_ctr();

    printf("Start counter...");
    ioctl(file_desc, IOCTL_START_PMU_CTR);
    printf("Done\n");

    read_pmu_ctr();

    printf("Delay...\n");
    for(i=0;i<4190;i++) dump[i] = i;

    read_pmu_ctr();

    read_tsc();
    return 0;
}
