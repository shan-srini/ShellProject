#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdbool.h>

// An idea to support ints as well as char* for variables
// have field in hashmap_pair as int val_int and char* val_string
// In hashmap_put and hashmap_get just check which type you're dealing with
// in put it would be checking the type of argument, in get you would be checking
// the fields val_int and val_string types and depending on which one returning that
// the issue with this is that the function wouldn't be following signature, so maybe
// returning the whole hashmap_pair would be the best solution? regardless I'm keeping
// it so that my nush can only store char* values

typedef struct hashmap_pair {
    char* key; // null terminated strings
    char* val; 
    bool used;
    bool tomb;
} hashmap_pair;

typedef struct hashmap {
    hashmap_pair** data;
    int size;
    int capacity;
} hashmap;

hashmap* make_hashmap();
void free_hashmap(hashmap* hh);
int hashmap_has(hashmap* hh, char* kk);
char* hashmap_get(hashmap* hh, char* kk);
void hashmap_put(hashmap* hh, char* kk, char* vv);
void hashmap_del(hashmap* hh, char* kk);
hashmap_pair hashmap_get_pair(hashmap* hh, int ii);
void hashmap_dump(hashmap* hh);

#endif
