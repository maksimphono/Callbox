#ifndef _SYSCALL_HASHMAP_H_
#define _SYSCALL_HASHMAP_H_

#include "defs.h"

typedef struct Hashmap_entry_t {
    char* syscall_name;
    u_int32_t syscall_num;
    struct Hashmap_entry_t* next;
} Hashmap_entry_t;

typedef struct Syscall_hashmap_t {
    Hashmap_entry_t** entries;
    size_t size;
} Syscall_hashmap_t;

Syscall_hashmap_t* new_Syscall_hashmap(size_t size);

void del_Syscall_hashmap(Syscall_hashmap_t* this);

size_t hash(char *str);

Hashmap_entry_t* new_Hashmap_entry(char* name, u_int32_t syscall_num);

void del_Hashmap_entry(Hashmap_entry_t* this);

void insert_Syscall_hashmap(Syscall_hashmap_t* this, char* name, u_int32_t syscall_num);

u_int32_t get_Syscall_hashmap(Syscall_hashmap_t* this, char* name);

#endif