
#include "reuse.h"

using namespace std;

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd<0){
        die("socket()");
    }
    int val=1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    struct sockaddr_in addr={};
    addr.sin_family=AF_INET;
    addr.sin_port=ntohs(1234);
    addr.sin_addr.s_addr=ntohl(0);

    int rv=bind(fd, (const sockaddr*)&addr, sizeof(addr));

    if(rv){
        die("bind");
    }

    fd_set_nb(fd);

    rv=listen(fd, SOMAXCONN);
    if(rv){
        die("failure in the listening ");
    }

    std::vector<Conn* > fd2conn;
    std::vector<struct pollfd> pollargs;

    while(true){
        pollargs.clear();
        struct pollfd pfd={fd, POLLIN, 0};

        pollargs.push_back(pfd);

        for(Conn *conn: fd2conn){
            if(!conn) continue;
            struct pollfd pfd ={conn->fd, POLLERR, 0};
            if(conn->want_read){
                pfd.events|=POLLIN;
            }

            if(conn->want_write){
                pfd.events|=POLLOUT;
            }
            pollargs.push_back(pfd);        
        }
        int rv=poll(pollargs.data(), (nfds_t)pollargs.size(), -1);
        if(rv<0 && errno==EINTR){
            continue;
        }
        if(rv<0) die(" poll() ");

        if(pollargs[0].revents){
            if(Conn *conn=handle_accept(fd)){
                if(fd2conn.size()<=(size_t)conn->fd){
                    fd2conn.resize(conn->fd+1);
                }
                assert(!fd2conn[conn->fd]);
                fd2conn[conn->fd]=conn;
            }
        }
        for (size_t i = 1; i < pollargs.size(); ++i)
        {
            uint32_t ready = pollargs[i].revents;

            if(ready==0){
                continue;
            }
            Conn *conn = fd2conn[pollargs[i].fd];
            if(ready & POLLIN){
                assert(conn->want_read);
                handle_read(conn);
            }

            if(ready & POLLOUT){
                assert(conn->want_write);
                handle_write(conn);
            }

            if((ready & POLLERR)||conn->want_close){
                (void)close(conn->fd);
                fd2conn[conn->fd]=nullptr;
                delete conn;
            }
        }        
    }
    return 0;
}


