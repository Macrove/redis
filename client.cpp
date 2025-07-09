#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <assert.h>
#include "utils.h"

#define K_MAX_BUF (32<<20)
#define K_MAX_ARGS (200*1000)
#define PORT 8080

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
    //msg("read_full:reading response");
    while(len>0){
        //msg("len:%d", len);
        ssize_t rv = read(fd, buf, len);
        //msg("rv:%d",rv);
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
int send_req(int fd, std::vector<std::string> &cmd){
    // write_all to the fd. if there's an error, log and return 
    if (cmd.size() > K_MAX_ARGS) {
        msg("send_req:Too many args");
        return -1;
    }
    uint32_t len = 4;
    for(auto &s:cmd){
        len+=s.size()+4;
    }
    if (len + 4 > K_MAX_BUF) {
        msg("send_req:Too big message");
        return -1;
    }
    if(cmd.size() > K_MAX_ARGS){
        msg("send_req:Too many args");
        return -1;
    }
    std::vector<uint8_t> wbuf;
    buf_append(wbuf, (uint8_t*)&len, 4);
    uint32_t nstr = cmd.size();
    buf_append(wbuf, (uint8_t*)&nstr, 4);
    for (auto &s : cmd) {
        uint32_t size = s.size();
        buf_append(wbuf, (uint8_t *)&size, 4);
        buf_append(wbuf, (uint8_t *)s.data(), size);
    }
    //for(int i=0;i<wbuf.size();i++){
        //std::cout << (int)wbuf[i] << " ";
    //}
    //std::cout << std::endl;
    if(write_all(fd, wbuf.data(), wbuf.size()) < 0){
        msg("send_req:failed to write request");
        return -1;
    }
    return 0;
}
int read_res(int fd){
    std::vector<uint8_t> res;
    res.resize(4);
    errno = 0;
    // read message body len
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
    if(len < 4){
        msg("read_res:Insufficient bytes");
        return -1;
    }
    res.clear();
    res.resize(4);
    // read status
    rv = read_full(fd, res.data(), 4);
    if(rv){
        msg("read() error");
        return rv;
    }
    uint32_t status = 0;
    memcpy(&status, res.data(), 4);
    msg("Return response status:%d\n", (int)status);
    if(len == 4) return 0;
    res.clear();
    res.resize(len - 4);
    // read message if present
    rv = read_full(fd,res.data(),len-4);
    for(int i=0;i<res.size() && i<100;i++){
        std::cout << (char)res[i];
    }
    std::cout << '\n';
    return 0;
}

int main(int argc, char* argv[]){
    if(argc<3){
        msg("Invalid args");
        return 1;
    }
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
    std::vector<std::string> cmd;
    for(int i=1;i<argc;i++){
        cmd.push_back(argv[i]);
        //msg("cmd.back():%s", cmd.back().c_str());
    }

    int32_t err = send_req(fd, cmd);
    if(err){
        msg("main:send_req failed");
        close(fd);
        return 1;
    }

    err = read_res(fd);
    if (err) {
        msg("main:read_res failed");
        close(fd);
        return 1;
    }
}
