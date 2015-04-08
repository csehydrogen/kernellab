//#include"my_dev.h"
#include<stdio.h>
#include<sys/ioctl.h>
#include<stdlib.h>

static int file_desc;
static int BUF_LEN;

int set_buflen(){
    int ret = ioctl(file_desc, IOCTL_SET_BUFFER_LENGTH);
    if(ret == SUCCESS) return SUCCESS;
    printf("Buffer length setting failed\n");
    exit(1);
}

int get_currpid(){
    int ret = ioctl(file_desc, IOCTL_GET_PID);
    if(ret >= 0) return ret;
    printf("Get_pid failed\n");
    exit(1);
}

char* get_currname(){
    char* buf;
    int ret;
    buf = malloc(BUF_LEN);
    ret = ioctl(file_desc, IOCTL_GET_NAME, buf);
    if(ret == SUCCESS) return buf;
    printf("Get_name failed\n");
    exit(1);
}

int move_parent(){
    int ret = ioctl(file_desc, IOCTL_MOVE_TO_PARENT);
    if(ret == SUCCESS) return ret;
    else return FAIL;
}

int move_current(){ /* init ftn */
    int ret = ioctl(file_desc, IOCTL_MOVE_TO_CURRENT);
    if(ret == SUCCESS) return ret;
    printf("Move_to_current failed\n");
    exit(1);
}

struct process{
    int pid;
    char* name;
};

struct process_list{
    struct process item;
    struct process_list *prev;
};

static int list_size;

static struct process_list *tail;

void init_list(){
    tail = NULL;
}

void add_item(int pid, char *name){
    if(tail == NULL){
        tail = malloc(sizeof(struct process_list));
        tail->prev = NULL;
    }
    else{
        struct process_list *tmp = malloc(sizeof(struct process_list));
        tmp->prev = tail;
        tail = tmp;
    }
    (tail->item).pid = pid;
    (tail->item).name = name;
    list_size ++;
}

void pop_item(){
    struct process_list *tmp = tail->prev;
    free(tail);
    tail = tmp;
    list_size --;
}

char* item_name(){
    return (tail->item).name;
}

int item_pid(){
    return (tail->item).pid;
}

int main(){
    int indent = 0;
    int iter = 0;

    list_size = 0;
    BUF_LEN = 127;
    file_desc = open(DEVICE_FILE_NAME, 0);
    init_list();
    move_current();

    set_buflen();
    do{
        int pid;
        char *name;
        int res;

        pid = get_currpid();
        name = get_currname();
        add_item(pid, name);
    }while(move_parent() != FAIL);

    while(list_size){
        int i, pid;
        char *name;
        for(i=0; i<indent; i++) printf(" ");
        if(iter){
            printf("\\-");
            indent += 2;
            iter ++;
        }

        pid = item_pid();
        name = item_name();

        printf("%s(%d)\n", name, pid);
        pop_item();
    }

    return 0;
}
