#ifndef _QUEUE_H_
#define _QUEUE_H_
#include <stdbool.h>
//queue element type
typedef char* Msg;
#define MAX_Q_ELEMENT 10000

typedef struct msgInfo{
    Msg msg;
    short MsgLen;
}Msg_Info;

typedef struct node{
    Msg_Info msg_info;
    struct node * next;
}Node;//自定义struct node 为Node类型。方便声明

typedef struct queue{
    Node * head;//指向队列头向的指针
    Node * tail;//指向队列尾向的指针
    unsigned int msg_num;
    //bool IsFull;
}Queue;

bool Init_Queue(Queue * qp);

bool Queue_Is_Full(Queue * qp);

bool Queue_Is_Emety(Queue * qp);

int Queue_Msg_Num(Queue * qp);

int Q_Put(Queue * qp,Msg msg,short num);//返回1，成功入队列；返回 0 ，队列满；返回-1 ，内存以满，需要释放内存！

short Q_Get(Queue * qp, Msg * msg);//成功将声明的新指针直接指向Queue中保存的Msg * 所指向的一块连续内存，返回长度（short），成功出队列；返回 0 ，队列为空；

void Q_Empty(Queue * qp);

bool Init_ThreadSafeQueue(Queue * qp);

int SQ_Put(Queue * qp,Msg msg,short num);//返回1，成功入队列；返回 0 ，队列满；返回-1 ，内存以满，需要释放内存！

short SQ_Get(Queue * qp, Msg * msg);

#endif