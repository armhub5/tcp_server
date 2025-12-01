#include "reuse.h"
#include<string>

using namespace std;

//const size_t cl_k_max_msg = 32<<20;


int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd<0){
        die("sockets");
    }

    struct sockaddr_in addr={};
    addr.sin_family=AF_INET;
    addr.sin_port=ntohs(1234);
    addr.sin_addr.s_addr=ntohl(INADDR_LOOPBACK);

    int rv=connect(fd, (const struct sockaddr*)&addr, sizeof(addr));
    if(rv){
        die("Connect . . . ");
    }

    std::vector<std::string> query_list{
        "hello...1", "hello...3", "hello...2","hello ...5",std::string(cl_k_max_msg, 'z'),
    };

    for(const std::string &s :query_list){
        int32_t err=send_req(fd, (uint8_t *)s.data(), s.size());
        if(err){
            goto L_DONE;
        }
    }

    for(size_t i=0;i<query_list.size();++i){
        int32_t err=read_res(fd);
        if(err){
            goto L_DONE;
        }
    }
L_DONE:
    close(fd);
    return 0;
}