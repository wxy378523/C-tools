#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "spinlock.h"
#include "queue.h"

static void CopyToNode(Msg pM,Node * pN,short num);
static short CopyToMsg(Msg *Copy_pM,Node * pN);

static void 
CopyToNode(Msg pM,Node * pN,short num){
    pN->msg_info .msg = pM;
    pN->msg_info .MsgLen = num;
}

static short 
CopyToMsg(Msg* Copy_pM,Node * pN){
    *Copy_pM = pN->msg_info.msg; //将指向外面的char 类型指针的二级指针传入，就可以直接修改外面的指针指向队列中储存的Msg 指向的内存，就不用发生copy，提高效率。
    return pN->msg_info.MsgLen;
}

bool 
Init_Queue(Queue * qp){
    qp->head = NULL;
    qp->tail = NULL;
    qp->msg_num=0;
    return true;
}

bool 
Queue_Is_Full(Queue * qp){
    return qp->msg_num == MAX_Q_ELEMENT;
}

bool 
Queue_Is_Emety(Queue * qp){
    if (qp->msg_num == 0)
        return true;
    return false;
}

int 
Queue_Msg_Num(Queue * qp){
    return qp->msg_num;
}

int 
Q_Put(Queue * qp,Msg msg,short num){ //返回1，成功入队列；返回 0 ，队列满；返回-1 ，内存以满，需要释放内存！
    Node * Newp;
    if (Queue_Is_Full(qp))
        return 0;
    Newp = (Node *)malloc(sizeof(Node));
    if (Newp == NULL)
        return -1;
    CopyToNode(msg,Newp,num);
    Newp->next=NULL;
    if (Queue_Is_Emety(qp))
        qp->head=Newp;
    else
        qp->tail->next=Newp;  //将原队尾->next指针指向Newp
    qp->tail=Newp;
    qp->msg_num++;

    return 1;
}

//msg 是二级指针
short 
Q_Get(Queue * qp, Msg * msg){ //成功将msg冲入缓冲，返回长度（short），成功出队列；返回 0 ，队列为空；
    if(Queue_Is_Emety(qp))
        return (short)0;
    short num;
    num = CopyToMsg(msg,qp->head);
    Node * tempp;
    tempp = qp->head;
    qp->head = qp->head->next;
    free(tempp);
    (qp->msg_num)--;
    if (Queue_Is_Emety(qp))
        qp->tail=NULL;
    return num;
}

void 
Q_Free(Queue * qp){
    Msg msg;
    while(!Queue_Is_Emety(qp))
        Q_Get(qp,&msg);
}