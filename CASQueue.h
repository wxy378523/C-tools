#ifndef _CASQUEUE_H_
#define _CASQUEUE_H_


typedef struct  {
    char * massage;
    int leagth;
}Q_msg;

typedef struct {
    struct {
        volatile unsigned int target;
        volatile unsigned int current;
    }head;

    struct {
        volatile unsigned int target;
        volatile unsigned int current;
    }tail;
    Q_msg * msg_queue ;
    int C_size;
    int C_mask;
    int C_element_num;
}C_queue;

C_queue * C_Init();

//if queue is full,return -1;
int 
C_push(C_queue * queue,char * msg,int lea);

int 
C_pop(C_queue * queue,Q_msg * need_msg);

int
C_free(C_queue * queue);

int
C_leagth(C_queue * queue);

#endif