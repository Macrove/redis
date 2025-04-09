#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <assert.h>

#define K_MAX_BUF (32<<20)
#define PORT 8080

void msg(const char* msg, ...){
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}
void die(const char* msg){
    fprintf(stderr, "%s\n", msg);
    std::abort();
}
void buf_append(std::vector<uint8_t> &buf, uint8_t *data, ssize_t len){
    buf.insert(buf.end(), data, data+len);
}

int write_all(int fd, uint8_t* data, size_t len){
    while(len>0){
        ssize_t rv = write(fd, data, len);
        if(rv<=0){
            msg("Write Failed");
            return -1;
        }
        assert(rv <= len);
        len-=(size_t)rv;
        data+=rv;
    }
    return 0;
}

int read_full(int fd, uint8_t* buf, size_t len){
    while(len>0){
        msg("len:%d", len);
        ssize_t rv = read(fd, buf, len);
        msg("rv:%d",rv);
        if(rv <= 0){
            msg("read failed");
            return -1;
        }
        assert(rv<=len);
        len-=(size_t)rv;
        buf+=rv;
    }
    return 0;
}
int send_req(int fd, uint8_t *data, size_t len){
    // write_all to the fd. if there's an error, log and return 
    if(len+4 > K_MAX_BUF){
        msg("Trying to send too big msg of len:%d. Limit is %d", len, K_MAX_BUF);
        return -1;
    }
    std::vector<uint8_t> req;
    buf_append(req, (uint8_t *)&len, 4);
    buf_append(req, data, len);
    if(write_all(fd, req.data(), req.size())){
        msg("write_all failed");
        return -1;
    };

    return 0;
}
int read_res(int fd){
    std::vector<uint8_t> res;
    res.resize(4);
    errno = 0;
    size_t rv = read_full(fd, res.data(), 4);
    if(rv<0){
        if (errno == 0) {
            msg("EOF");
        }
        else{
            msg("read() error");
        }
        return rv;
    }
    size_t len = 0;
    memcpy(&len, res.data(), 4);
    if(len > K_MAX_BUF){
        msg("msg too long");
        return -1;
    }
    res.clear();
    res.resize(len);
    rv = read_full(fd, res.data(), len);
    if(rv){
        msg("read() error");
        return rv;
    }
    // do something
    fprintf(stdout, "Return result: size:%d, data:%.*s\n", (int)res.size(), int(100<len?100:len), res.data());
    return 0;
}

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0){
        die("socket failed");
    }
    struct sockaddr_in server_addr={};
    server_addr.sin_port = htons(PORT);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    socklen_t addrlen = sizeof(server_addr);

    int rv = connect(fd, (const struct sockaddr *)&server_addr, addrlen);
    if (rv) {
        die("Connect failed");
    }

    std::vector<std::string> requests = {
        "request1", 
        "request2",
        "request3",
        std::string(K_MAX_BUF-4,'z'),
        //std::string(100,'z'),
        "request4"
    };

    for(auto &req: requests){
        printf("send_req - len:%d, req: %.*s\n", (int)req.size(),int(req.size() < 100 ? req.size():100), req.data());
        if(send_req(fd, (uint8_t *)req.data(), req.size())){
            msg("send_req failed");
            close(fd);
            return 1;
        }
    }

    std::cout << requests.size() << std::endl;
    for(size_t i=0;i<requests.size();i++){
        if(read_res(fd)){
            msg("read_res failed");
            close(fd);
            return 1;
        }
    }
}
