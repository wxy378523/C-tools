#ifndef _INTHASHMAP_H_
#define _INTHASHMAP_H_
#include <stdbool.h>
#include "spinlock.h"

typedef struct element_node{
    struct element_node * Next;
    struct spinlock lock;
    void * value;
    int key;
    int hashcode;
    bool define;     //True；表示以赋值     False：表示未赋值过
}int_table_element;

typedef struct{
    int_table_element * Table;
    struct spinlock lock;
    int table_size;
    int element_num;
}int_HashTable;

int_HashTable * int_hashmap_init();

/*
    init size 16,resize size * 2;
    if resize false return -2,put new key if false return -1;successfully return 1;if IntHashmap has zhe key,return 2,value update successfully,and old value point was free successfully!
    if resize false,automatic Rollback old array.
*/     
int int_put(int_HashTable * Hash_Table,int key,void *value);

bool int_element_equals(int key, int num);

bool int_haskey(int_HashTable* Hash_Table,int key);

void * int_get(int_HashTable * Hash_Table,int key);

bool int_table_free(int_HashTable ** Hash_Table);

int int_table_print(int_HashTable * Hash_Table);

//success return element,false return NULL;
void * int_delete_key(int_HashTable * Hash_Table,int key);

#endif