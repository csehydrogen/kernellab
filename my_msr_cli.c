#include <stdio.h>
#include <sys/ioctl.h>
#include "my_dev.h"

static int file_desc;
static int dump[65536];

unsigned long long read_pmu_ctr(){
    unsigned long long tmp;
    ioctl(file_desc, IOCTL_READ_PMU_CTR, &tmp);
    return tmp;
}

unsigned long long read_tsc(){
    unsigned long long tmp;
    ioctl(file_desc, IOCTL_READ_TSC, &tmp);
    return tmp;
}

void delay(int n){
    int i;
    for(i = 0; i < n; i++) dump[i] = i;
}

void test2(){
    int i;

    printf("[TSC test]\n");
    for(i = 0; i < 16; i++){
        delay(1024);
        printf("Delay... TSC : %016llx\n", read_tsc());
    }
}

void test1(){
    int i;
    
    printf("[Scale test]\n");

    printf("Counter start...");
    ioctl(file_desc, IOCTL_START_PMU_CTR);
    printf("Done\n");

    for(i = 1; i <= 65536; i *= 2){
        ioctl(file_desc, IOCTL_RESET_PMU_CTR);
        delay(i);
        printf("Assign %5d elements... %016llx counted\n", i, read_pmu_ctr());
    }
}

void test0(){
    unsigned long long before, after;

    printf("[START, STOP, RESET test]\n");

    ioctl(file_desc, IOCTL_START_PMU_CTR);

    printf("- Counter started\n");
    printf("Before delay : %016llx\n", before = read_pmu_ctr());
    delay(1024);
    printf("After  delay : %016llx\n", after = read_pmu_ctr());
    printf("%s\n", before < after ? "PASS" : "FAIL");

    ioctl(file_desc, IOCTL_STOP_PMU_CTR);

    printf("- Counter stopped\n");
    printf("Before delay : %016llx\n", before = read_pmu_ctr());
    delay(1024);
    printf("After  delay : %016llx\n", after = read_pmu_ctr());
    printf("%s\n", before == after ? "PASS" : "FAIL");

    ioctl(file_desc, IOCTL_RESET_PMU_CTR);

    printf("- Counter resetted\n");
    printf("Current : %016llx\n", before = read_pmu_ctr());
    printf("%s\n", before == 0 ? "PASS" : "FAIL");
}

int main(){
    unsigned long select_bit = EVT_UOPS_ISSUED | USR_FLG;

    file_desc = open(DEVICE_FILE_NAME, 0);

    printf("Select event %08lx(UOPS_ISSUED)...", select_bit); 
    ioctl(file_desc, IOCTL_SELECT_PMU_EVT, select_bit);
    printf("Done\n"); 

    test0();
    test1();
    test2();

    return 0;
}
