#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

static void die(const char* message){
    perror(message);
    std::abort();
}
void msg_errno(const char* msg){
    fprintf(stderr, "[errno:%d] %s\n",errno, msg);
}
void msg(const char* msg, ...){
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}

