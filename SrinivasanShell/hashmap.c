
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <bsd/string.h>
#include <string.h>

#include "hashmap.h"
// attribution to Nat Tuck lecture notes, used suggestions and help from in class implementation

int
hash(char* key, long nn)
{
    int hh = 0;
    for(int i = 0; key[i]; ++i) {
        hh = hh * 67 + key[i];
    }
    
    return hh & (nn-1);
}

hashmap*
make_hashmap_presize(int nn)
{
    hashmap* hm = calloc(1, sizeof(hashmap));
    hm->data = calloc(nn, sizeof(hashmap_pair*));
    hm->size = 0;
    hm->capacity = nn;
    
    return hm;
}

hashmap*
make_hashmap()
{
    return make_hashmap_presize(4);
}

hashmap_pair*
make_hashmap_pair(char* key, char* val) 
{
    hashmap_pair* hp = malloc(sizeof(hashmap_pair));
    hp->key = strdup(key);
    hp->val = strdup(val);
    hp->used = 0;
    hp->tomb = 1;
    return hp;
}

void
free_pair(hashmap_pair* hp)
{
    if(hp) {
        free(hp->key);
        free(hp->val);
        free(hp);
    }
}

void
free_hashmap(hashmap* hh)
{
    //for(int i = 0; i<hh->capacity; ++i) {
      // free_pair(hh->data[i]);
    //}
    free(hh->data);
    free(hh);
}

int
hashmap_has(hashmap* hh, char* kk)
{
    char* val = hashmap_get(hh, kk);
    if(strlen(kk) != strlen(val)) {
        //false
        return 0;
    }
    if(strcmp(hashmap_get(hh, kk), kk) != 0) {
        //true
        return 1;
    }
    else {
        //false
        return 0;
    }
}

char*
hashmap_get(hashmap* hh, char* kk)
{
    // key kk.
    // Note: return -1 for key not found.
    int i = hash(kk, hh->capacity);
    //for(int z = i; i < hh->size; z++) {
        if(hh->data[i] != 0) {
        //if(strcmp(hh->data[z]->key, kk) == 0) {
            return hh->data[i]->val;
        //}
    }
    return 0;
}

void
grow_hash(hashmap* hh)
{
    int nn = hh->capacity;
    hashmap_pair** data = hh->data;

    hh->capacity = 2 * nn;
    hh->data = calloc(hh->capacity, sizeof(hashmap_pair*));
    hh->size = 0;

    for(int i = 0; i<nn; ++i) {
        if(data[i]) {
            hashmap_put(hh, data[i]->key, data[i]->val);
        
            //free(data[i]);
        }
    }
    free(data);
}


void
hashmap_put(hashmap* hh, char* kk, char* vv)
{
    // for the key 'kk', replacing any existing value
    // for that key.
    //
    if(hh->size > hh->capacity) {
        grow_hash(hh);
    }
    int i = hash(kk, hh->capacity);
    //struct hashmap_pair ex = {val: vv, used: 1, tomb: 0};
    //strlcpy(ex.key, kk, 4);
    //ex.key[4] = '\0';
    //hh->data[i] = &ex;
    hh->data[i]=make_hashmap_pair(kk, vv);

    hh->size += 1;
}

void
hashmap_del(hashmap* hh, char* kk)
{
    // Remove any value associated with
    // this key in the map.
    for(int i = 0; i<hh->capacity; i++) {
        if(hh->data[i]) 
         if(strcmp(hh->data[i]->key, kk) == 0) {
            hh->data[i]->tomb = 1;
            hh->data[i]->used = 0;
            hh->data[i]->val = 0;
        } else {
            continue;
        }
     }
}

hashmap_pair
hashmap_get_pair(hashmap* hh, int ii)
{
    // Get the {k,v} pair stored in index 'ii'.
    return *(hh->data[ii]);
}

void
hashmap_dump(hashmap* hh)
{
    printf("== hashmap dump ==\n");
    // Print out all the keys and values currently
    // in the map, in storage order. Useful for debugging.
    for(int i = 0; i<hh->capacity; i++){
        if(hh->data[i]){
            printf("%ld and val is %c", hh->data[i]->val, hh->data[i]->key[0]);
        }
        else {
            puts("null");
        }
    }
}

