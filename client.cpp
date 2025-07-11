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

#define K_MAX_BUF (4096)
#define K_MAX_ARGS (200*1000)
#define PORT 8080

enum {
    TAG_NIL = 0,
    TAG_ERR = 1,
    TAG_STR = 2,
    TAG_INT = 3,
    TAG_DBL = 4,
    TAG_ARR = 5
};

enum ERR_CODES {
    UNKNOWN_OPERATION = 0,
    MSG_TOO_BIG = 1
};

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

int read_full(int fd, char* buf, size_t len){
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

int32_t print_response(char *res, size_t size){
    if(size <= 0){
        msg("print_res: size < 0");
        return -1;
    }
    switch(res[0]){
        case TAG_NIL:
            msg("(nil)\n");
            return 1;
        case TAG_ERR:
            if(1 + 8 < size){
                msg("print_res: bad res in TAG_ERR. size: %d", size);
                return -1;
            }
            {
                uint32_t code, len;
                memcpy(&code, &res[1], 4);
                memcpy(&len, &res[1 + 4], 4);
                if(size < 1 + 4 + 4 + len){
                    msg("print_res: bad str res in TAG_ERR. size: %d", size);
                    return -1;
                }
                printf("(err) %d %.*s\n", code, len, &res[1 + 8]);
                return 1+8+len;
            }
        case TAG_INT:
            if(size < 1 + 4){
                msg("print_res: bad res in TAG_INT. size: %d", size);
                return -1;
            }
            {
                uint32_t val;
                memcpy(&val, &res[1], 4);
                msg("(int) %d\n", val);
                return 1 + 4;

            }
        case TAG_DBL:
            if(size < 1 + 8){
                msg("print_res: bad res in TAG_DBL. size: %d", size);
                return -1;
            }
            {
                uint32_t val;
                memcpy(&val, &res[1], 4);
                msg("(dbl) %g\n", val);
                return 1 + 4;
            }
        case TAG_STR:
            if(size < 1 + 4){
                msg("print_res: bad res in TAG_STR. size: %d", size);
                return -1;
            }
            {
                uint32_t len;
                memcpy(&len, &res[1 + 4], 4);
                printf("(str) %.*s\n", len, &res[1 + 4]);
                return 1 + 4 + len;
            }
        case TAG_ARR:
            if(size < 1 + 4){
                msg("print_res: bad_res in TAG_ARR. size: %d", size);
                return -1;
            }
            {
                uint32_t arr_len;
                memcpy(&arr_len, &res[1 + 4], 4);
                uint32_t str_len;
                uint32_t arr_bytes = 1 + 4;
                printf("(arr) ");
                for(int i=0;i<arr_len;i++){
                    if(size < arr_bytes + 4){
                        msg("print_res: bad_res in str_len TAG_ARR. size: %d arr_bytes: %d", size, arr_bytes);
                        return -1;
                    }
                    memcpy(&str_len, &res[arr_bytes], 4);
                    if(size < arr_bytes + 4 + str_len){
                        msg("print_res: bad res in str TAG_ARR. size: %d, arr_bytes: %d, str_len: %d", size, arr_bytes, str_len);
                        return -1;
                    }
                    printf(", %.*s", str_len, &res[arr_bytes + 4]);
                    arr_bytes += 4 + str_len;
                }
                printf("\n");
                return  arr_bytes;
            }
        default:
            msg("print_res: bad_tag. tag: %c", res[0]);
            return -1;
    }
}
int read_res(int fd){
    //std::vector<uint8_t> res;
    char rbuf[4 + K_MAX_BUF];
    errno = 0;
    // read message body len
    //size_t rv = read_full(fd, res.data(), 4);
    size_t rv = read_full(fd, rbuf, 4);
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
    memcpy(&len, rbuf, 4);
    if(len > K_MAX_BUF){
        msg("msg too long");
        return -1;
    }
    // read status
    rv = read_full(fd, rbuf, len);
    if(rv){
        msg("read() error");
        return rv;
    }
    rv = print_response(rbuf, len);
    if(rv <= 0){
        msg_errno("bad response");
        return -1;
    }
    return rv;

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

    rv = read_res(fd);
    if (rv < 0) {
        msg("main:read_res failed");
        close(fd);
        return 1;
    }
}
