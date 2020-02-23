#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <fcntl.h>

#include "svec.h"
#include "tokenize.h"
#include "hashmap.h"

//Functions that need hoisting to the top, should prob move all definitions here eventually 
int execute_normal(svec* tokens);
void eval(svec* tokens);

//File scope variables
//redeeming my messy hashmap implementation from hw04, and using it here for variables
hashmap* vars;

//function to convert every token which has $ (indicating its a var) to its value
void
resolve(svec* tokens) {
    char* current;
    for(int i = 0; i<tokens->size; ++i) {
        current = svec_get(tokens, i);
        if(current[0] == '$') {
            int len = strlen(current);
            char key[len];
            strncpy(key, &current[1], len);
            //memcpy(key, current[1], len-1);
            //key[len] = '\0';
            //puts(key);
            char* newVal = hashmap_get(vars, key);
            svec_put(tokens, i, newVal);
        }
    }
}

void
execute(char* cmd)
{
    int cpid;

    if ((cpid = fork())) {
        // parent process
        printf("Parent pid: %d\n", getpid());
        printf("Parent knows child pid: %d\n", cpid);

        // Child may still be running until we wait.

        int status;
        waitpid(cpid, &status, 0);

        printf("== executed program complete ==\n");

        printf("child returned with wait code %d\n", status);
        if (WIFEXITED(status)) {
            printf("child exited with exit code (or main returned) %d\n", WEXITSTATUS(status));
        }
    }
    else {
        // child process
        printf("Child pid: %d\n", getpid());
        printf("Child knows parent pid: %d\n", getppid());

        for (int ii = 0; ii < strlen(cmd); ++ii) {
            if (cmd[ii] == ' ') {
                cmd[ii] = 0;
                break;
            }
        }

        // The argv array for the child.
        // Terminated by a null pointer.
        char* args[] = {cmd, "one", 0};

        printf("== executed program's output: ==\n");

        execvp(cmd, args);
        printf("Can't get here, exec only returns on error.");
    }
}

//executes with given svec* starting finding args at start and ending at end
//changing function to return int of status, for OR and AND implementations...
int
execute_normal(svec* tokens) {
    int cpid;
    if((cpid = fork())) {
        int waiter;
        waitpid(cpid, &waiter, 0);
        //this should return the status code of cpid
        //according to man pages and sample code in starter code
        return WEXITSTATUS(waiter);
    }
    else {
        char* args[tokens->size+1];
        for(int i = 0; i < tokens->size; i++) {
            args[i] = svec_get(tokens, i);
        }
        args[tokens->size] = 0;
        int returned = execvp(svec_get(tokens, 0), args);
        if(returned != 0) {
            printf("nush: %s is not a command, unrecognized input\n", svec_get(tokens, 0));
            exit(0);
        }
    }
}

void
execute_cd(svec* tokens) {
    int rv;
    char* dir = svec_get(tokens, 1);
    rv = chdir(dir);
    if(rv == -1) {
        //mimicking bash :)
        puts("nush: cd: no such directory");
    }
}

//Checks if tokens has operator, should probably move to svec functions instead
//returns index for given operator
int
has_operator(svec* tokens, char* op) {
    //char opString[2];
    //opString[op];
    //opString[1] = 0;
    for(int i = 0; i<tokens->size; ++i) {
        if(strcmp(svec_get(tokens, i), op) == 0) {
            //puts(op);
            //return index
            return i;
        }
    }
    //false
    return -1;
}


//executes a command with ; the int op is where the ; is
void
eval_semi(svec* tokens, int op) {
    //for the for loop later
    char* temp;

    //allocated on heap...free
    svec* left = make_svec();
    svec* right = make_svec();

    for(int i = 0; i<op; ++i) {
        svec_push_back(left, tokens->data[i]);
    }

    for(int i = op+1; i<tokens->size; ++i) {
        svec_push_back(right, tokens->data[i]);
    }

    eval(left);
    eval(right);

    free_svec(left);
    free_svec(right);
}

//executes a command with || and op is where the || is
void
eval_or(svec* tokens, int op) {
    int cpid;
    int left_eval;
    if((cpid = fork())) {
        int waiter;
        //attribution: Nat Tuck, using code given in starter code with
        //WEXIT and waitpid
        waitpid(cpid, &waiter, 0);
        if(WEXITSTATUS(waiter) != 0){
            svec* right = make_svec();
            for(int i = op+1; i<tokens->size; ++i) {
                svec_push_back(right, svec_get(tokens, i));
            }
            
            eval(right);
            free_svec(right);
        }
    }
    else {
        svec* left = make_svec();
        for(int i = 0; i<op; ++i) {
            svec_push_back(left, tokens->data[i]);
        }
        left_eval = execute_normal(left);
        free_svec(left);
        //my execute_normal forks on it's own, so here I am terminating this fork
        //and exiting with the status code of the execution...
        //furthermore, _exit avoids stdio streams being flushed
        //as Prof Tuck has noted, this is appropriate as I am creating an
        //extra child process and propogating an exit code...
        _exit(left_eval);
    }
}

//executes a command with && and op is where the && is
void
eval_and(svec* tokens, int op) {
    int cpid;
    int left_eval;
    if((cpid = fork())) {
        int waiter;
        //attribution: Nat Tuck, using code given in starter code with
        //WEXIT and waitpid
        waitpid(cpid, &waiter, 0);
        if(WEXITSTATUS(waiter) == 0) {
            svec* right = make_svec();
            for(int i = op+1; i<tokens->size; ++i) {
                svec_push_back(right, svec_get(tokens, i));
            }
            
            eval(right);
            free_svec(right);
        }
    }
    else {
        svec* left = make_svec();
        for(int i = 0; i<op; ++i) {
            svec_push_back(left, tokens->data[i]);
        }
        left_eval = execute_normal(left);
        free_svec(left);
        //my execute_normal forks on it's own, so here I am terminating this fork
        //and exiting with the status code of the execution...
        //furthermore, _exit avoids stdio streams being flushed
        //as Prof Tuck has noted, this is appropriate as I am creating an
        //extra child process and propogating an exit code...
        _exit(left_eval);
    }
}

//evaluate in the background
//approach is fork and don't wait in parent
void
eval_bg(svec* tokens, int op) {
    int cpid;
    if((cpid = fork())) {

    }
    else {
        svec* left = make_svec();
        for(int i = 0; i<op; ++i) {
            svec_push_back(left, svec_get(tokens, i));
        }
        eval(left);
        free_svec(left);
        
        svec* right = make_svec();
        for(int i = op+1; i<tokens->size; ++i) {
            svec_push_back(right, svec_get(tokens, i));
        }
        eval(right);
        free_svec(right);
        //extra child process, since calling eval results in extra
        //fork at execute_normal function, so don't want to flush stdin...
        _exit(0);
    }
}

//evaluate the redirect in operator using file descriptors
void
eval_redirect_in(svec* tokens, int op) {
    int cpid;
    if((cpid = fork())) {
        int waiter;
        wait(&waiter);
    }
    else {
        //attribution: Nat Tuck lecture 10
        int fd = open(svec_get(tokens, op+1), O_RDONLY, 0644);
        close(0);
        dup(fd);
        close(fd);
        svec* cmd = make_svec();
        for(int i = 0; i<op; ++i) {
            svec_push_back(cmd, svec_get(tokens, i));
        }
        eval(cmd);
        //extra child process, since calling eval results in extra
        //fork at execute_normal function, so don't want to flush stdin...
        _exit(0);
    }

    
}

//evaluate the redirect out operator using file descriptors
void
eval_redirect_out(svec* tokens, int op) {
    int cpid;
    if((cpid = fork())) {
        int waiter;
        wait(&waiter);
    }
    else {
        //attribution: Nat Tuck lecture 10
        //leaving out append, because it doesn't seem necessary here
        int fd = open(svec_get(tokens, op+1), O_CREAT | O_WRONLY, 0644);
        close(1);
        dup(fd);
        close(fd);
        svec* cmd = make_svec();
        for(int i = 0; i<op; ++i) {
            svec_push_back(cmd, svec_get(tokens, i));
        }
        eval(cmd);
        //extra child process, since calling eval results in extra
        //fork at execute_normal function, so don't want to flush stdin...
        _exit(0);
    }
}

//last but not least, eval pipe
//approaching with the 1 child solution, seems more straightforward than forking twice
//fork once: close stdout dup with my stdout, then eval left of | operator (int op)
//then exit and in parent which waited for exit close stdin replace with my stdin
//eval everything to right of | op
void
eval_pipe(svec* tokens, int op) {
    int cpid;
    //attribution Nat Tuck lecture 10
    int pipe_fds[2];
    pipe(pipe_fds);
    int pipe_read = pipe_fds[0];
    int pipe_write = pipe_fds[1];
    int c_cpid;

    if((cpid=fork())) {
        int waiter;
        waitpid(cpid, &waiter, 0);
        close(0);
        dup(pipe_read);
        close(pipe_write);
        close(pipe_read);
        svec* right = make_svec();
        for(int i = op+1; i<tokens->size; ++i) {
            svec_push_back(right, svec_get(tokens, i));
        }
        eval(right);
        free_svec(right);
        //puts("in parent");
    }
    else {
        close(1);
        dup(pipe_write);
        close(pipe_read);
        close(pipe_write);
        //puts("in child");
        svec* left = make_svec();
        for(int i = 0; i<op; ++i) {
            svec_push_back(left, svec_get(tokens, i));
        }
        eval(left);
        free_svec(left);
        exit(0);
    }
}

//assign variables by storing them in the filescope variable "vars" which is hashmap*
void
eval_var_assign(svec* tokens, int op) {
    assert((op==1 && tokens->size > 1)&&"Nush: Seems like there's intent of variable assignment, but not valid input");
    char* key = svec_get(tokens, op-1);
    char* val = svec_get(tokens, op+1);
    hashmap_put(vars, key, val);
    //puts(hashmap_get(vars, key));
}

/*
//subshell process from start paren to end paren, then evaluate rest
void
eval_sub_shell(svec* tokens, int startP, int endP) {
    int cpid;
    printf("%ld", startP);
    printf("%ld", endP);

    if((cpid=fork())) {
        int waiter;
        waitpid(cpid, &waiter, 0);
        svec* rest;
        if(tokens->size>endP+1)
        for(int i = endP+1; i<tokens->size; ++i) {
            svec_push_back(rest, svec_get(tokens, i));
        }
        if(rest->size > 0) {
        eval(rest);
        }
        else {
            return;
        }
    }
    else {
        svec* sub;
        for(int i = startP+1; i<endP; ++i) {
            svec_push_back(sub, svec_get(tokens, i));
        }
        eval(sub);
        free_svec(sub);
        //exit(0);
    }
}*/

void
eval(svec* tokens) {
    if(tokens->size == 0) {
        return;
    }
    
    int op;

    //cd should be the first arg in svec this is okay here
    if(strcmp(svec_get(tokens, 0), "cd") == 0) {
        execute_cd(tokens);
        return;
    }

    //exit should be the only command this is okay
    if(strcmp(svec_get(tokens, 0), "exit") == 0) {
        exit(0);
    }
    
    //need a way to scan svec for an operator
    //precedence goes || & || && & > <
    if((op = has_operator(tokens, ";")) != -1) {
        eval_semi(tokens, op);
    }
    else if((op = has_operator(tokens, "||")) != -1) {
        eval_or(tokens, op);
    }
    else if((op = has_operator(tokens, "&&")) != -1) {
        eval_and(tokens, op); 
    }
    //checking | and & after || and && ensures no mistakes
    else if((op = has_operator(tokens, "&")) != -1) {
        eval_bg(tokens, op);
    }
    else if((op = has_operator(tokens, "|")) != -1) {
        eval_pipe(tokens, op); 
    } 
    else if((op = has_operator(tokens, "<")) != -1) {
        eval_redirect_in(tokens, op); 
    }
    else if((op = has_operator(tokens, ">")) != -1) {
        eval_redirect_out(tokens, op);
    }
    else if ((op = has_operator(tokens, "=")) != -1) {
        eval_var_assign(tokens, op);
    }
    /*else if((op = has_operator(tokens, "(")) != -1) {
        int endP = has_operator(tokens, ")");
        eval_sub_shell(tokens, op, endP);
    }*/
    else {
        execute_normal(tokens);
    }
    
    /*
    //!!!debugging just printing out tokens
    for(int i = 0; i<tokens->size; i++) {
        puts(tokens->data[i]);
    }
    */
}

int
main(int argc, char* argv[])
{
    char cmd[256];
    
    //init the hashmap of vars
    vars = make_hashmap();

    // interactive mode
    if (argc == 1) {
        while(1) {
            printf("nush$ ");
            fflush(stdout);
            char* status = fgets(cmd, 256, stdin);
            if(!status) {
                puts("");
                return 0;
            }
            svec* tokens = tokenize(cmd);
            //resolves variables to values before eval
            resolve(tokens);
            eval(tokens);
            //execute step maybe? or maybe eval will just call it
            free_svec(tokens);
          }
    }
    // script mode
    else {
        FILE* script = fopen(argv[1], "r");
        //clever trick I saw online to show a custom error message with assert
        assert((script != 0)&&"Script not found");
        
        while(fgets(cmd, 256, script) != NULL) {
            svec* tokens = tokenize(cmd);
            //resolves variables to values before eval
            resolve(tokens);
            eval(tokens);
            free_svec(tokens);
            fflush(stdout);
        }
        fclose(script);
    }

    free_hashmap(vars);
    return 0;
}
