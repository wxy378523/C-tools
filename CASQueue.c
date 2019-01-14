#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "CASqueue.h"

#define QSZ   ((unsigned int)1024*2)
#define QMSK  (QSZ - 1)

C_queue * 
C_Init(){
    C_queue * queue = (C_queue*) calloc(1, sizeof(C_queue));
    Q_msg * array = (Q_msg*)malloc(QSZ * sizeof(Q_msg));
    if (!array)
        return NULL;
    memset (array,0,QSZ * sizeof(Q_msg));
    queue ->msg_queue = array ;
    queue -> C_size = QSZ;
    queue -> C_mask = QMSK;
    queue ->C_element_num =0;
    return queue;
}

//head.current 为了标记当前确认将 值 存入对应的下角标,通俗讲就是该标记之前的元素以确认处理完。34，35，36行并不是原子操作，所以在38行确认执行完，才可以pop 这个下角标的元素。
//head.target 表示唯一的线程通过原子操作 获取唯一一个合法的脚标，然后等到之前的元素都确认 以处理完毕，就可以执行39行，将当前的脚标标记为 已完成。

int
C_push(C_queue * queue,char * msg, int lea){
    unsigned int head, tail ,next ,mask,index;

    mask = queue ->C_mask;
    do{
        head = queue -> head.target;
        tail = queue -> tail.current;
        if ((head - tail )>mask)
            return -1;
        next = head +1;
    }while(!__sync_bool_compare_and_swap(&(queue ->head.target),head,next)); //head = head.target刚开始是一个可以推入数据的下脚标，进行原子操作，head保证只有一个线程可以拿到这个唯一的下脚步，往里面插入数据。
    //用这个唯一 head 下角标，取的数组的元素，赋值
    index = head & mask;
    queue -> msg_queue[index].massage = msg ;
    queue -> msg_queue[index].leagth =lea ;
    
    while(!__sync_bool_compare_and_swap(&(queue ->head.current),head,next))
    return 1;
}

int 
C_pop(C_queue * queue,Q_msg * need_msg){
    unsigned int head, tail ,next ,mask,index;

    mask = queue -> C_mask;
    do{
        tail = queue -> tail . target;
        head = queue ->head.current;
        if ((head - tail) < 1U)
            return -1;
        next = tail + 1;
    }while(!__sync_bool_compare_and_swap(&(queue ->tail.target),tail,next));
    index = tail & mask;
    need_msg -> massage = ((queue -> msg_queue)+index) -> massage;
    need_msg -> leagth = ((queue -> msg_queue)+index) -> leagth;

    while(!__sync_bool_compare_and_swap(&queue ->tail.current,tail,next))
    return 1;
}

int
C_free(C_queue * queue){
    free(queue ->msg_queue);
    queue ->msg_queue = NULL;
    free(queue);
    queue = NULL;
    return 1;
}

int
C_leagth(C_queue * queue){
    return (queue ->head.current - queue ->tail.current);
}

/*这有点类似你去银行取钱的时候，一进门先得去一台机器上领个号码，然后银行再按照这个号码的先后顺序为客户服务。
我们称这个号码为序号(sequence)，比如你领到的是5号，如上图，但是银行目前只处理完了2号客户之前的客户
(包括2号客户，即上图中的 head.second)，3, 4号客服都是正在办理中的，而5号是你新领到的。
跟现实银行不太一样地方是，这里只要你一领到序号，窗口就立刻开始为你服务了，而各个窗口（各个线程）办理完成的时间是不定的，
可能4号窗口先办理完，然后再是3号窗口，最后5号窗口，也就是你领到的号码。还有一点跟现实银行不一样的地方，
就是不管3, 4, 5窗口谁先办理完成，都必须按照序号的大小来结束办理过程，即4号窗口即使完成了，也要等3号窗口完成了以后才算完成，否则只能等3号窗口完成。
如果5号窗口先办理完毕，也必须等4号窗口完成，才能算最终完成。因为只有这么依次完成，我们才好挪动 head.second 到最新成功提交数据的那个位置，
如果不按照这个顺序，跳着挪动 head.second，那么 head.second 就乱套了。
*/