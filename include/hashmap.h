#ifndef _SYSCALL_HASHMAP_H_
#define _SYSCALL_HASHMAP_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "defs.h"

#define _DEFINE_GENERIC_HASHMAP( \
    Prefix,     /* prefix to methods like "static" or "inline" */ \
    Name,       /* name of the class, will be used with structure and methods */ \
    Hash_F,     /* hash function */ \
    Key_T,      /* type of the key */ \
    Val_T,      /* type of the value */ \
    Cpy_key_F,  /* key copying function */ \
    Cpy_val_F,  /* value copying function */ \
    Cmp_F,      /* key comparison function */ \
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
Prefix const Val_T Name##_NOT_FOUND = (const Val_T)Not_found_V; \
\
Prefix Name* new_##Name(size_t size) { \
    Name* new_hashmap_instance = (Name*)malloc(sizeof(Name)); \
    new_hashmap_instance->size = size; \
    new_hashmap_instance->entries = (Name##_entry_t**)calloc(size, sizeof(Name##_entry_t*)); \
\
    return new_hashmap_instance; \
} \
\
Prefix Name##_entry_t* new_##Name##_entry(Key_T key, Val_T val) { \
    Name##_entry_t* new_instance = (Name##_entry_t*)malloc(sizeof(Name##_entry_t)); \
    /* copying key and value (I assume these functions should B 2-argument macros) */ \
    Cpy_key_F(new_instance->_key, key); \
    Cpy_val_F(new_instance->_val, val); \
\
    new_instance->next = 0x0; \
\
    return new_instance; \
} \
Prefix void del_##Name##_entry(Name##_entry_t* this) {  \
    if (this == 0x0) return; \
\
    /* Delete key and value using provided destructors (provide EMPTY_F if no destruction is needed) */ \
    Key_D(this->_key); \
    Val_D(this->_val); \
\
    free(this); \
}\
Prefix void del_##Name(Name* this) { \
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
Prefix void set_##Name(Name* this, Key_T key, Val_T val) { \
    if (this == 0x0) return; \
    size_t id = Hash_F(key) % this->size; \
\
    Name##_entry_t* new_instance; \
\
    if (this->entries[id] != 0x0) { \
        Name##_entry_t* current = this->entries[id]; \
        while (current != 0x0 && !Cmp_F(current->_key, key)) { \
            current = current->next; \
        } \
        if (current != 0x0) { \
            Val_D(current->_val); \
            Cpy_val_F(current->_val, val); \
            return; \
        } \
    } \
    new_instance = new_##Name##_entry(key, val); \
    new_instance->next = this->entries[id]; \
    this->entries[id] = new_instance; \
} \
Prefix Val_T get_##Name(Name* this, Key_T key) { \
    if (this == 0x0) return Name##_NOT_FOUND; \
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
Prefix void drop_##Name(Name* this, Key_T key) { \
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
#define STR_CPY(__dest, __src) __dest = strdup(__src)
#define ASSIGN(__dest, __src) __dest = __src
#define SIMPLE_EQ(__a, __b) (__a == __b)
#define SIMPLE_HASH(__n) (size_t)__n
#define STR_CMP(__a, __b) (strcmp(__a, __b) == 0)

/* just define regular hashmap */
#define DEFINE_HASHMAP(Name,Hash_F,Key_T,Val_T,Cpy_key_F,Cpy_val_F,Cmp_F,Key_D,Val_D,Not_found_V) \
    _DEFINE_GENERIC_HASHMAP(,Name,Hash_F,Key_T,Val_T,Cpy_key_F,Cpy_val_F,Cmp_F,Key_D,Val_D,Not_found_V)
/* define static hashmap */
#define DEFINE_STATIC_HASHMAP(Name, Hash_F,Key_T,Val_T,Cpy_key_F,Cpy_val_F,Cmp_F,Key_D,Val_D,Not_found_V) \
    _DEFINE_GENERIC_HASHMAP(static inline, Name,Hash_F,Key_T,Val_T,Cpy_key_F,Cpy_val_F,Cmp_F,Key_D,Val_D,Not_found_V)
/* define regular hashmap where key and values are simple types that don't need removal */ 
#define DEFINE_SIMPLE_HASHMAP(Name,Key_T,Val_T,Not_found_V) \
    _DEFINE_GENERIC_HASHMAP(,Name,SIMPLE_HASH,Key_T,Val_T,ASSIGN,ASSIGN,SIMPLE_EQ,EMPTY_F,EMPTY_F,Not_found_V)
/* same as above but static */ 
#define DEFINE_SIMPLE_STATIC_HASHMAP(Name,Key_T,Val_T,Not_found_V) \
    _DEFINE_GENERIC_HASHMAP(static inline,Name,SIMPLE_HASH,Key_T,Val_T,ASSIGN,ASSIGN,SIMPLE_EQ,EMPTY_F,EMPTY_F,Not_found_V)


#endif