#include "IntHashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TABLE_SIZE 16
#define RATIO_NUM 0.75
#define INCR_RATIO 2
/*读，并没有保证线程安全，写，自旋锁需要验证*/

static unsigned int 
int_Hash(int key){
    return key;
}

//将hash值 根据TABLE_SIZE -1取模，映射成下脚标。
static int 
indexFor(int hash,int leagth){
    return hash & (leagth-1);
}

int_HashTable * 
int_hashmap_init(){
    int index;
    int_HashTable * Hash_Table;
    Hash_Table = malloc(sizeof(int_HashTable));
    if (Hash_Table == NULL)
        exit(1);
    Hash_Table ->element_num=0;
    Hash_Table ->table_size = TABLE_SIZE;
    SPIN_INIT(Hash_Table);
    int_table_element * array_Node;
    array_Node = malloc(sizeof(int_table_element) * (Hash_Table ->table_size));
    if (array_Node == NULL)
        exit(1);
    memset(array_Node,0,sizeof(int_table_element) * (Hash_Table ->table_size));
    for (index=0 ; index < Hash_Table -> table_size; index++){
        SPIN_INIT(array_Node+index);
    }
    Hash_Table ->Table = array_Node;
    return Hash_Table;
}

bool 
int_element_equals(int  key,int target){
    return key == target;
}

bool 
int_haskey(int_HashTable* Hash_Table,int key){
    int_table_element * node;
    int index = (int)indexFor(int_Hash(key),Hash_Table ->table_size);
    int_table_element * backet = Hash_Table ->Table + index;
    if (!backet || !backet -> define)
        return NULL;
    if (int_element_equals(key,backet ->key) && (backet -> define)){
        return true;
    }
    node = backet ->Next;
    while (node){
        if (int_element_equals(key,node ->key) ){
            return true;
        }
        node = node -> Next;
    }
    return false;
}

static int_table_element * 
If_has_key_return(int key,int_table_element * backet){
    int_table_element * node;
    if (!backet || !backet -> define)
        return NULL;
    if (int_element_equals(key,backet ->key) && (backet -> define)){
        return backet;
    }
    node = backet -> Next;
    while (node){
        if (int_element_equals(key,node ->key)){
            return node;
        }
        node = node -> Next;
    }
    return NULL;
}

static int_table_element * 
Init_Node(int key,void * value,int hash){
    int_table_element * newNode=malloc(sizeof(int_table_element));
    if (!newNode)
        return NULL;
    newNode -> Next = NULL;
    newNode -> key = key;
    newNode -> value = value;
    newNode -> define = true;
    newNode -> hashcode = hash;
    return newNode;
}

static int 
put_new_key_into_Backet(int_table_element * backet,int key,void *value,int hash){
    if (backet -> define){
        int_table_element * newNode=Init_Node(key,value,hash);
        if (!newNode)
            return -1;
        if ((backet -> Next)!= NULL)
            backet=backet -> Next;
        backet -> Next = newNode;
        return 1;
    }
    else{
        backet -> Next = NULL;
        backet -> key = key;
        backet -> value = value;
        backet -> define = true;
        backet -> hashcode = hash;
        return 1;
    }
    return -1;
}

static bool 
err_needto_free(int_table_element * table,int table_size){
    int index;
    int_table_element * curnode;
    int_table_element * need_free_node;
    for (index=0 ; index < table_size; index++){
        curnode = table + index;
        if (!curnode -> define)
            continue;

        int_table_element * next_node = curnode ->Next;
        curnode -> Next = NULL;
        while(next_node){
            need_free_node = next_node;
            next_node = next_node -> Next;
            free(need_free_node);
            need_free_node = NULL;
        }
    }
    free(table);
    table = NULL;
    return true;
}

//自动扩容
static int_table_element * 
table_resize(int_HashTable * Hash_Table){
    if (!Hash_Table)
        return NULL;
    int index,Hash,size,TempIndex,oldsize;
    oldsize = Hash_Table ->table_size;
    size = oldsize * INCR_RATIO;
    int_table_element * oldTable = Hash_Table -> Table;  //原数组
    int_table_element * newTable = malloc(size*sizeof(int_table_element));  //新数组
    if (!newTable)
        return NULL;
    memset(newTable,0,sizeof(int_table_element) * size);

    for (index=0 ; index < size; index++){
        SPIN_INIT(newTable+index);
    }
    
    int_table_element * oldTable_node ;
    int_table_element * newTable_node;
    int_table_element * need_free_node;
    for (index=0 ; index < Hash_Table -> table_size; index++){
        oldTable_node = oldTable + index;
        SPIN_LOCK(oldTable_node);
        if (!oldTable_node -> define)
            continue;
        Hash = oldTable_node -> hashcode;
        if (!(Hash & oldsize)){
            TempIndex = index;
            newTable_node = newTable + TempIndex;
            int n = put_new_key_into_Backet(newTable_node,oldTable_node -> key,oldTable_node -> value,Hash);
            if (n <0){
                err_needto_free(newTable,size);   //创建新数组的过程中出现错误要回滚，把申请的内存第归释放掉
                return NULL;
            }
        }
        else{
            TempIndex = (size >> 1) +index;
            newTable_node = newTable + TempIndex;
            int n = put_new_key_into_Backet(newTable_node,oldTable_node -> key,oldTable_node -> value,Hash);
            if (n <0){
                err_needto_free(newTable,size);   //创建新数组的过程中出现错误要回滚，把申请的内存第归释放掉
                return NULL;
            }
        }

        int_table_element * old_next_node = oldTable_node ->Next;
        oldTable_node -> Next = NULL;
        while(old_next_node){
            Hash = old_next_node ->hashcode;
            if (!(Hash & oldsize)){
                TempIndex = index;
                newTable_node = newTable + TempIndex;
                int n = put_new_key_into_Backet(newTable_node,old_next_node -> key,old_next_node -> value,Hash);
                if (n <0){
                    err_needto_free(newTable,size);   //创建新数组的过程中出现错误要回滚，把申请的内存第归释放掉
                    return NULL;
                }
            }
            else{
                TempIndex = (size >> 1) +  index;
                newTable_node = newTable + TempIndex;
                int n = put_new_key_into_Backet(newTable_node,old_next_node -> key,old_next_node -> value,Hash);
                if (n <0){
                    err_needto_free(newTable,size);   //创建新数组的过程中出现错误要回滚，把申请的内存第归释放掉
                    return NULL;
                }
            }
            need_free_node = old_next_node;
            old_next_node = old_next_node -> Next;
            free(need_free_node);
            need_free_node = NULL;
        }
        SPIN_UNLOCK(oldTable_node);
    }
    Hash_Table -> Table = NULL;
    free(oldTable);
    oldTable = NULL;
    return newTable;
}

int 
int_put(int_HashTable * Hash_Table,int key,void *value){
    if (!Hash_Table)
        return 0;
    if((sizeof(key) != 4) || key <0)
        return 0;

    SPIN_LOCK(Hash_Table);
    if ((Hash_Table->element_num) >=(int) (Hash_Table -> table_size * RATIO_NUM)){
        int_table_element * newTable = table_resize(Hash_Table);
        if (newTable){
            Hash_Table ->Table = newTable;
            Hash_Table ->table_size = Hash_Table ->table_size * INCR_RATIO;
        }
        else
            return -2;
    }
    SPIN_UNLOCK(Hash_Table);

    int hash = int_Hash(key);
    int index = (int)indexFor(hash,Hash_Table ->table_size);
    int_table_element * IndexNode = (Hash_Table ->Table)+index;
    SPIN_LOCK(IndexNode);
    int_table_element * oldNode = If_has_key_return(key,IndexNode);
    if (oldNode){
        free(oldNode -> value);
        oldNode -> value = value;
    }
    else{
          //证明没有此key
        int n =put_new_key_into_Backet(IndexNode , key , value , hash);
        if (n < 0){
            SPIN_UNLOCK(IndexNode);
            return -1;
        }
        SPIN_LOCK(Hash_Table);
        Hash_Table ->element_num = Hash_Table ->element_num +1;
        SPIN_UNLOCK(Hash_Table);
    }
    SPIN_UNLOCK(IndexNode);
    return 1;
}

void * 
int_get(int_HashTable * Hash_Table,int key){
    int index = (int)indexFor(int_Hash(key),Hash_Table ->table_size);
    int_table_element * node = Hash_Table ->Table + index;
    node = If_has_key_return(key,node);
    if(!node)
        return NULL;
    return node -> value;
}

bool 
int_table_free(int_HashTable ** Hash_Table_point){
    int_table_element * curnode;
    int_table_element * need_free_node;
    int_table_element * table = (*Hash_Table_point) ->Table;
    int index;

    for (index=0 ; index < (*Hash_Table_point) -> table_size; index++){
        curnode = table + index;
        if (!curnode -> define)
            continue;

        free(curnode -> value);
        curnode ->value =NULL;

        int_table_element * next_node = curnode ->Next;
        curnode -> Next = NULL;
        while(next_node){
            need_free_node = next_node;
            next_node = next_node -> Next;
            free(need_free_node -> value);
            need_free_node -> value =NULL;
            free(need_free_node);
            need_free_node = NULL;
        }
    }
    (*Hash_Table_point) -> Table = NULL;
    free(table);
    table = NULL;
    free((*Hash_Table_point));
    (*Hash_Table_point) = NULL;
    return true;
}

int 
int_table_print(int_HashTable * Hash_Table){
    int index;
    int_table_element * element = Hash_Table -> Table;
    int_table_element * node ;
    for (index = 0; index < Hash_Table -> table_size; index ++){
        node = element + index;
        while(node){
            if ((node -> key) >=0 && (node -> define))
                printf("[%d : %p]；\n",(node -> key),node ->value);
            node = node -> Next;
        }
    }
}

void * 
int_delete_key(int_HashTable * Hash_Table,int key){
    int index = (int)indexFor(int_Hash(key),Hash_Table ->table_size);
    int_table_element * node = Hash_Table ->Table + index;
    int_table_element * need_free_node, * next_node;
    void * value;
    if (!node || !node -> define)
        return NULL;
    SPIN_LOCK(node);
    if (int_element_equals(key,node ->key) && (node -> define)){
        value = node -> value;
        next_node = node -> Next;
        if (next_node){
            node -> value = next_node -> value;
            node -> key = next_node -> key;
            node -> Next = next_node -> Next;
            node -> hashcode = next_node -> hashcode;
            free(next_node);
            next_node = NULL;
        }
        else{
            node -> value = NULL;
            node -> key =0;
            node -> Next = NULL;
            node -> hashcode = 0;
            node -> define =false;
        }

        SPIN_LOCK(Hash_Table);
        Hash_Table -> element_num= Hash_Table -> element_num -1;
        SPIN_UNLOCK(Hash_Table);

        // if (Hash_Table -> element_num == 0)
        //         int_table_free(&Hash_Table);
        SPIN_UNLOCK(node);
        return value;
    }
    int_table_element * old_node = node;
    while (node -> Next ){
        if (int_element_equals(key,node ->Next ->key)){
            need_free_node = node -> Next;
            node -> Next = node -> Next -> Next;
            value = need_free_node -> value;
            free(need_free_node);

            SPIN_LOCK(Hash_Table);
            Hash_Table -> element_num= Hash_Table -> element_num -1;
            SPIN_UNLOCK(Hash_Table);

            SPIN_UNLOCK(old_node);
            return value;
        }
        node = node -> Next;
    }
    SPIN_UNLOCK(old_node);
    return NULL;
}