#include "StrHashmap.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TABLE_SIZE 16
#define RATIO_NUM 0.75
#define INCR_RATIO 2

//求str的hash值。
unsigned int BKDRHash(char *str){
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;
    while (*str){
        hash = hash * seed + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}

//将hash值 根据TABLE_SIZE -1取模，映射成下脚标。
static int indexFor(int hash,int leagth){
    return hash & (leagth-1);
}

HashTable * hash_table_init(){
    HashTable * Hash_Table;
    Hash_Table = malloc(sizeof(HashTable));
    if (Hash_Table == NULL)
        exit(1);
    Hash_Table ->element_num=0;
    Hash_Table ->table_size = TABLE_SIZE;
    table_element * array_Node;
    array_Node = malloc(sizeof(table_element) * (Hash_Table ->table_size));
    if (array_Node == NULL)
        exit(1);
    memset(array_Node,0,sizeof(table_element) * (Hash_Table ->table_size));
    Hash_Table ->Table = array_Node;
    return Hash_Table;
}

HashTable* hash_table_init_ByDefineSize(int my_size){
    return NULL;
}

bool str_equals(char * key,char * str2){
    return !strcmp(key,str2);
}

bool haskey(HashTable* Hash_Table,char * key){
    int index = (int)indexFor(BKDRHash(key),Hash_Table ->table_size);
    table_element * node = Hash_Table ->Table + index;
    while (node){
        if (str_equals(key,node ->key)){
            return true;
        }
        node = node -> Next;
    }
    return false;
}

static table_element * If_has_key_return(char * key,table_element* node){
    if (!node || !node -> define)
        return NULL;
    while (node){
        if (str_equals(key,node ->key)){
            return node;
        }
        node = node -> Next;
    }
    return NULL;
}

static table_element * Init_Node(char * key,void * value,int hash){
    table_element * newNode=malloc(sizeof(table_element));
    if (!newNode)
        return NULL;
    newNode -> Next = NULL;
    newNode -> key = key;
    newNode -> value = value;
    newNode -> define = false;
    newNode -> hashcode = hash;
    return newNode;
}

static int put_new_key_into_Backet(table_element * backet,char * key,void *value,int hash){
    if (backet -> define){
        table_element * newNode=Init_Node(key,value,hash);
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

static bool err_needto_free(table_element * table,int table_size){
    int index;
    table_element * curnode;
    table_element * need_free_node;
    for (index=0 ; index < table_size; index++){
        curnode = table + index;
        if (!curnode -> define)
            continue;

        table_element * next_node = curnode ->Next;
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
static table_element * table_resize(HashTable * Hash_Table){
    if (!Hash_Table)
        return NULL;
    int index,Hash,size,TempIndex,oldsize;
    oldsize = Hash_Table ->table_size;
    size = oldsize * INCR_RATIO;
    table_element * oldTable = Hash_Table -> Table;  //原数组
    table_element * newTable = malloc(size*sizeof(table_element));  //新数组
    if (!newTable)
        return NULL;
    memset(newTable,0,sizeof(table_element) * size);
    table_element * oldTable_node ;
    table_element * newTable_node;
    table_element * need_free_node;
    for (index=0 ; index < Hash_Table -> table_size; index++){
        oldTable_node = oldTable + index;
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

        table_element * old_next_node = oldTable_node ->Next;
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
    }
    Hash_Table -> Table = NULL;
    free(oldTable);
    oldTable = NULL;
    return newTable;
}

int put(HashTable * Hash_Table,char * key,void *value){
    if (!Hash_Table)
        return 0;
    if(!key)
        return 0;
    if ((Hash_Table->element_num) >=(int) (Hash_Table -> table_size * RATIO_NUM)){
        table_element * newTable = table_resize(Hash_Table);
        if (newTable){
            Hash_Table ->Table = newTable;
            Hash_Table ->table_size = Hash_Table ->table_size * INCR_RATIO;
        }
        else
            return -2;
    }
    int hash = BKDRHash(key);
    int index = (int)indexFor(hash,Hash_Table ->table_size);
    table_element * IndexNode = (Hash_Table ->Table)+index;
    table_element * oldNode = If_has_key_return(key,IndexNode);
    if (oldNode){
        oldNode -> value = value;
        return 1;
    }
    else{
          //证明没有此key
        int n =put_new_key_into_Backet(IndexNode , key , value , hash);
        if (n < 0)
            return -1;
        Hash_Table ->element_num = Hash_Table ->element_num +1;
        return 1;
    }
}

void * get(HashTable * Hash_Table,char * key){
    int index = (int)indexFor(BKDRHash(key),Hash_Table ->table_size);
    table_element * node = Hash_Table ->Table + index;
    node = If_has_key_return(key,node);
    if(!node)
        return NULL;
    return node -> value;
}

bool table_free(HashTable ** Hash_Table_point){
    table_element * curnode;
    table_element * need_free_node;
    table_element * table = (*Hash_Table_point) ->Table;
    int index;
    for (index=0 ; index < (*Hash_Table_point) -> table_size; index++){
        curnode = table + index;
        if (!curnode -> define)
            continue;

        table_element * next_node = curnode ->Next;
        curnode -> Next = NULL;
        while(next_node){
            need_free_node = next_node;
            next_node = next_node -> Next;
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

int table_print(HashTable * Hash_Table){
    int index;
    int value;
    table_element * element = Hash_Table -> Table;
    table_element * node ;
    for (index = 0; index < Hash_Table -> table_size; index ++){
        node = element + index;
        while(node){
            if (node -> value)
                value = *((int *)node ->value);
            char * str = node -> key;
            if (str)
                printf("[%s : %d]；\n",str,value);
            node = node -> Next;
        }
    }
}

void * delete_key(HashTable ** t_table,char * key){
    HashTable * Hash_Table = *t_table;
    int index = (int)indexFor(BKDRHash(key),Hash_Table ->table_size);
    table_element * node = Hash_Table ->Table + index;
    table_element * need_free_node, * next_node;
    void * value;
    if (!node || !node -> define)
        return NULL;
    if (str_equals(key,node ->key)){
        value = node -> value;
        next_node = node -> Next;
        if (next_node){
            node -> value = next_node -> value;
            node -> key = next_node -> key;
            node -> Next = next_node -> Next;
            node -> hashcode = next_node -> hashcode;
            Hash_Table -> element_num= Hash_Table -> element_num -1;
            free(next_node);
            next_node = NULL;
        }
        else{
            node -> value = NULL;
            node -> key =NULL;
            node -> Next = NULL;
            node -> hashcode = 0;
            node -> define =false;
            Hash_Table -> element_num= Hash_Table -> element_num -1;
        }
        if (Hash_Table -> element_num == 0)
                table_free(t_table);
        return value;
    }
    while (node -> Next ){
        if (str_equals(key,node ->Next ->key)){
            need_free_node = node -> Next;
            node -> Next = node -> Next -> Next;
            value = need_free_node -> value;
            free(need_free_node);
            Hash_Table -> element_num= Hash_Table -> element_num -1;
            if (Hash_Table -> element_num == 0)
                table_free(t_table);
            return value;
        }
        node = node -> Next;
    }
    return NULL;
}

int element_Num(HashTable * Hash_Table){
    return Hash_Table -> element_num;
}