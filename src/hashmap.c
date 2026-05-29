#include "../include/hashmap.h"
#include <string.h>
#include <stdlib.h>

Syscall_hashmap_t* new_Syscall_hashmap(size_t size) {
    Syscall_hashmap_t* new_hashmap_instance = (Syscall_hashmap_t*)malloc(sizeof(Syscall_hashmap_t));

    new_hashmap_instance->size = size;
    new_hashmap_instance->entries = (Hashmap_entry_t**)malloc(size * sizeof(Hashmap_entry_t*));

    return new_hashmap_instance;
}

void del_Syscall_hashmap(Syscall_hashmap_t* this) {
    if (this == 0x0) return;

    Hashmap_entry_t* current = 0x0, *next = 0x0;

    for (size_t i = 0; i < this->size; i++) {
        current = this->entries[i];
        while (current != 0x0) {
            next = current->next;
            del_Hashmap_entry(current);
            current = next;
        }
    }

    free(this->entries);
    free(this);
}

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