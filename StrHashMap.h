#ifndef _STRHASHMAP_H_
#define _STRHASHMAP_H_
#include <stdbool.h>

typedef struct arr_element_node{
    struct arr_element_node * Next;
    char * key;
    void * value;
    int hashcode;
    bool define;     //True；表示以赋值     False：表示未赋值过
}table_element;

typedef struct {
    table_element * Table;
    int table_size;
    int element_num;
}HashTable;

HashTable * hash_table_init();

HashTable * hash_table_init_ByDefineSize(int my_size);

/*
    init size 16,resize size * 2;
    if resize false return -2,put new key if false return -1;successfully return 1;
*/      
int put(HashTable * Hash_Table,char * key,void *value);

bool str_equals(char * key,char * str2);

bool int_equals(int * key, int * num);

bool haskey(HashTable* Hash_Table,char * key);

void * get(HashTable * Hash_Table,char * key);

bool table_free(HashTable ** Hash_Table);

int table_print(HashTable * Hash_Table);

//success return element,false return NULL;
void * delete_key(HashTable ** Hash_Table,char * key);

int element_Num(HashTable * Hash_Table);

#endif
// 非 线程安全