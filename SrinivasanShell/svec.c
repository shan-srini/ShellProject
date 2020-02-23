/* This file is lecture notes from CS 3650, Fall 2018 */
/* Author: Nat Tuck */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "svec.h"
svec*
make_svec()
{
    svec* sv = malloc(sizeof(svec));
    sv->data = malloc(2 * sizeof(char*));
    sv->size = 0;
    sv->capacity = 2;
    // correctly allocate and initialize data structure
    return sv;
}

void
free_svec(svec* sv)
{
    // free all allocated data
    int index = 0;
    for(int i = 0; i<sv->size; i++) {
        if(sv->data[index]) {
            free(sv->data[index]);
            index = index + 1;
        }
    }
    free(sv->data);
    free(sv);
}

char*
svec_get(svec* sv, int ii)
{
    assert(ii >= 0 && ii < sv->size);
    return sv->data[ii];
}

void
svec_put(svec* sv, int ii, char* item)
{
    assert(ii >= 0 && ii < sv->capacity);
    // insert item into slot ii
    // Consider ownership of string in collection.
    sv->data[ii] = strdup(item);
}

void
svec_push_back(svec* sv, char* item) //char* item was passed at one point
{
    int ii = sv->size;
    int oldCap = sv->capacity;
    if(sv->size >= sv->capacity) {
        sv->data = (char**) realloc(sv->data, oldCap * 2 * sizeof(char*));
        sv->capacity = oldCap * 2;
    }
    
    // expand vector if backing erray
    sv->size = ii + 1;
    svec_put(sv, ii, item);
}

void
svec_swap(svec* sv, int ii, int jj)
{
    //Swap the items in slots ii and jj
    char* temp = sv->data[ii];
    sv->data[ii] = sv->data[jj];
    sv->data[jj]=temp;
}

