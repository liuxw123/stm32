#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct _hashmap_node {
    void* value;
    int key;
} hashmap_node;

typedef struct _hashmap_table {
    int size;
    struct _hashmap_node** list;
} hashmap;

hashmap* hashmap_init(int size);
void hashmap_destory(hashmap* obj);

void* hashmap_put(hashmap* obj, int key, void* value);
void* hashmap_get(hashmap* obj, int key);

#endif