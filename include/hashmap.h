#ifndef _SYSCALL_HASHMAP_H_
#define _SYSCALL_HASHMAP_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "defs.h"

// TODO: implement universal hashmap using marcos, that can be used with any type
//       provide custom types for keys and values
//       accept custom function for comparison of keys, copying the key, copying the value, removing the key, removing the value
//       add "del_entry"(or something) method, that will remove entry from the hashmap

#define DEFINE_HASHMAP( \
    Name,       /* name of the class, will be used with structure and methods */ \
    Key_T,      /* type of the key */ \
    Val_T,      /* type of the value */ \
    Cpy_key_F,  /* key copying function */ \
    Cpy_val_F,  /* value copying function */ \
    Cmp_F,      /* key comparison function */ \
    Hash_F,     /* hash function */ \
    Key_D,      /* key destructor */ \
    Val_D,      /* value destructor */ \
    Not_found_V /* if value wasn't found in the hashmap */ \
) \
    typedef struct Name##_entry_t { \
        Key_T _key; \
        Val_T _val; \
        struct Name##_entry_t* next; \
    } Name##_entry_t; \
    typedef struct Name { \
        Name##_entry_t** entries; \
        size_t size; \
    } Name;\
    const Val_T Name##_NOT_FOUND = (const Val_T)Not_found_V; \
\
Name* new_##Name(size_t size) { \
    Name* new_hashmap_instance = (Name*)malloc(sizeof(Name)); \
    new_hashmap_instance->size = size; \
    new_hashmap_instance->entries = (Name##_entry_t**)malloc(size * sizeof(Name##_entry_t*)); \
\
    return new_hashmap_instance; \
} \
\
Name##_entry_t* new_##Name##_entry(Key_T key, Val_T val) { \
    Name##_entry_t* new_instance = (Name##_entry_t*)malloc(sizeof(Name##_entry_t)); \
    /* copying key and value (I assume these functions should B 2-argument macros) */ \
    Cpy_key_F(new_instance->_key, key); \
    Cpy_val_F(new_instance->_val, val); \
\
    new_instance->next = 0x0; \
\
    return new_instance; \
} \
void del_##Name##_entry(Name##_entry_t* this) {  \
    if (this == 0x0) return; \
\
    /* Delete key and value using provided destructors (provide EMPTY_F if no destruction is needed) */ \
    Key_D(this->_key); \
    Val_D(this->_val); \
\
    free(this); \
}\
void del_##Name(Name* this) { \
    if (this == 0x0) return; \
\
    Name##_entry_t* current = 0x0, *next = 0x0;\
\
    for (size_t i = 0; i < this->size; i++) { \
        current = this->entries[i]; \
        while (current != 0x0) { \
            next = current->next; \
            del_##Name##_entry(current); \
            current = next; \
        } \
    } \
\
    free(this->entries); \
    free(this); \
} \
void insert_##Name(Name* this, Key_T key, Val_T val) { \
    if (this == 0x0) return; \
    size_t id = Hash_F(key) % this->size; \
\
    Name##_entry_t* new_instance; \
\
    if (this->entries[id] != 0x0) { \
        new_instance = new_##Name##_entry(key, val); \
        new_instance->next = this->entries[id]; \
        this->entries[id] = new_instance; \
    } else { \
        this->entries[id] = new_##Name##_entry(key, val); \
    } \
} \
Val_T get_##Name(Name* this, Key_T key) { \
    if (this == 0x0) return (Val_T)0x0; \
\
    size_t id = Hash_F(key) % this->size; \
\
    Name##_entry_t* current = this->entries[id]; \
\
    while (current != 0x0 && !Cmp_F(current->_key, key)) { \
        current = current->next; \
    } \
\
    if (current != 0x0) { \
        return current->_val; \
    } else { \
        return Name##_NOT_FOUND; \
    } \
} \
void drop_##Name(Name* this, Key_T key) { \
    if (this == 0x0) return; \
\
    size_t id = Hash_F(key) % this->size; \
\
    Name##_entry_t* current = this->entries[id]; \
    Name##_entry_t* prev = this->entries[id]; \
\
    while (current != 0x0 && !Cmp_F(current->_key, key)) { \
        prev = current; \
        current = current->next; \
    } \
\
    if (current == 0x0) { \
        return; \
    } \
\
    if (current == this->entries[id]) { \
        this->entries[id] = current->next; \
        del_##Name##_entry(current); \
    } else { \
        prev->next = current->next; \
        del_##Name##_entry(current); \
    } \
}\

#define EMPTY_F(...)

//void del_int(int n) {printf("Del: %d\n", n);}
//void del_char(char c) {printf("Del: %c\n", c);}

size_t hash(char *str) {
    u_int32_t hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return (size_t)hash;
}

#define STR_CPY(__dest, __src) __dest = strdup(__src)
#define ASSIGN(__dest, __src) __dest = __src
#define SIMPLE_EQ(__a, __b) (__a == __b)
#define STR_CMP(__a, __b) (strcmp(__a, __b) == 0)

size_t hash_int(int n) {
    return n;
}

DEFINE_HASHMAP(str_int_hashmap_t, char*, int, STR_CPY, ASSIGN, STR_CMP, hash,free,EMPTY_F, -99)
DEFINE_HASHMAP(int_char_HM, int, char, ASSIGN, ASSIGN, SIMPLE_EQ, hash_int,EMPTY_F,EMPTY_F, '@');

int main() {
    str_int_hashmap_t* hm = new_str_int_hashmap_t(2);
    int_char_HM* int_chat = new_int_char_HM(4);
    
    for (int i = 0; i < 14; i++) {
        insert_int_char_HM(int_chat, i, i + 97);
    }

    drop_int_char_HM(int_chat, 10); // 
    drop_int_char_HM(int_chat, 6);
    for (int i = 14; i < 20; i++) {
        insert_int_char_HM(int_chat, i, i + 97);
    }
    drop_int_char_HM(int_chat, 3);
    //drop_int_char_HM(int_chat, 667);
    //drop_int_char_HM(int_chat, 665);
    //drop_int_char_HM(int_chat, -1);
    drop_int_char_HM(int_chat, 13);

    for (int i = 0; i < 20; i++) {
        char c = get_int_char_HM(int_chat, i);
        if (c != '@') {
            printf("%d : %c\n", i, c);
        }
    }    


    insert_str_int_hashmap_t(hm, "one", 1);
    insert_str_int_hashmap_t(hm, "two", 2);
    insert_str_int_hashmap_t(hm, "tree", 3);

    drop_str_int_hashmap_t(hm, "three");
    drop_str_int_hashmap_t(hm, "two");
    drop_str_int_hashmap_t(hm, "t");
    drop_str_int_hashmap_t(hm, "one");

    del_str_int_hashmap_t(hm);
    del_int_char_HM(int_chat);
    return 0;
}

/*
size_t hash(char *str) {
    u_int32_t hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return (size_t)hash;
}

Hashmap_entry_t* new_Hashmap_entry(char* name, u_int32_t syscall_num) {
    Hashmap_entry_t* new_instance = (Hashmap_entry_t*)malloc(sizeof(Hashmap_entry_t));
    new_instance->syscall_name = (char*)malloc((strlen(name) + 1) * sizeof(char));
    strcpy(new_instance->syscall_name, name);
    new_instance->syscall_num = syscall_num;

    return new_instance;
}

void del_Hashmap_entry(Hashmap_entry_t* this) {
    if (this == NULL) return;

    free(this->syscall_name);
    free(this);
}

void insert_Syscall_hashmap(Syscall_hashmap_t* this, char* name, u_int32_t syscall_num) {
    if (this == 0x0) return;
    size_t id = hash(name) % this->size;

    Hashmap_entry_t* new_instance;

    if (this->entries[id] != 0x0) {
        new_instance = new_Hashmap_entry(name, syscall_num);
        new_instance->next = this->entries[id];
        this->entries[id] = new_instance;
    } else {
        this->entries[id] = new_Hashmap_entry(name, syscall_num);
    }
}

u_int32_t get_Syscall_hashmap(Syscall_hashmap_t* this, char* name) {
    if (this == 0x0) return (u_int32_t)-1;

    size_t id = hash(name) % this->size;

    Hashmap_entry_t* current = this->entries[id];

    while (current != 0x0 && strcmp(current->syscall_name, name) != 0) {
        current = current->next;
    }

    if (current != 0x0) {
        return current->syscall_num;
    } else {
        return HM_NOT_FOUND;
    }
}


Syscall_hashmap_t* new_Syscall_hashmap(size_t size);

void del_Syscall_hashmap(Syscall_hashmap_t* this);

size_t hash(char *str);

Hashmap_entry_t* new_Hashmap_entry(char* name, u_int32_t syscall_num);

void del_Hashmap_entry(Hashmap_entry_t* this);

void insert_Syscall_hashmap(Syscall_hashmap_t* this, char* name, u_int32_t syscall_num);

u_int32_t get_Syscall_hashmap(Syscall_hashmap_t* this, char* name);

*/
#endif