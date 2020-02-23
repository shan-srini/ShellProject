#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "tokenize.h"
#include "svec.h"

// is a char an operator
int
is_operator(char text) {
    //note: ( and ) are not really operators, but adding them
    //for subshell
    //char ops[8] = {'<', '>', '|', '&', ';', '=', '(', ')'};
    char ops[6] = {'<', '>', '|', '&', ';', '='}; 
    int i = 0;
    while(i<6) {
        if(ops[i] == text) {
            return 1;
        }
        ++i;
    }
    return 0;
}

//is this char an input that can be built on
//example is sort hello.txt -l all these chars except ' ' would pass
int
is_input(char text) {
    if(isalpha(text)) {
        return 1;
    }

    if(isdigit(text)) {
        return 1;
    }

    int other_amt = 6;
    char other[other_amt];
    other[0] = '.';
    other[1] = '-';
    other[2] = '/';
    other[3] = '_';
    other[4] = '$';
    other[5] = '"';

    int ii = 0;
    while(ii<other_amt) {
        if(text == other[ii]) {
            return 1;
        }
        ++ii;
    }
    return 0;
}

//return a word
char*
read_word(const char* text, int ii) {
    int index = 0;
    if(text[index + ii] == '"') {
        ++index;
        while(is_input(text[index+ii]) || text[index+ii] == ' ') {
            if(text[index+ii] == '"') {
                ++index;
                break;
            }
            ++index;
        }
    }
    else {
        while(is_input(text[index+ii])) {
            if(is_operator(text[index+ii])) {
                break;
            }
            ++index;
        }
    }
    char* word = malloc(index + 1);
    memcpy(word, text + ii, index);
    word[index]=0;
    return word;
}

svec*
tokenize(const char* text) {
    svec* list = make_svec();
    int nn = 0;
    if(text)
        nn = strlen(text);
    int index = 0;

    while(index < nn) {
        if(isspace(text[index])) {
            index = index + 1; 
            continue;
        }

        if(is_operator(text[index])) {
            if(text[index]=='|' && index+1<nn && text[index+1]=='|') {
                svec_push_back(list, "||");
                index+=2;
                continue;
            }
            if(text[index]=='&' && index+1<nn && text[index+1]=='&') {
                svec_push_back(list, "&&");
                index+=2;
                continue;
            }
            else {
                char op[2];
                op[0]=text[index];
                op[1]=0;
                svec_push_back(list, op);
                index+=1;
                continue;
            }
        }

        if(is_input(text[index])) {
            char* word = read_word(text, index);
            svec_push_back(list, word);
            index+=strlen(word);
            free(word);
            continue;
        }

        ++index;
    }
    return list;
}
