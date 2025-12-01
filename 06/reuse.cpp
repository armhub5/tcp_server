
#include "reuse.h"
#include <string.h>

static void msg(const char *msg){
    fprintf(stderr, "%s \n", msg);
}

static void msg_errno(const char* msg){
    fprintf(stderr, "[errno :: %d ]%s \n",errno, msg);
}

static void die(const char* msg){
    fprintf(stderr, "[errno :: %d ]%s \n",errno, msg);
    abort();
}


//set the file descriptor for the socket as a non blocking
static void fd_set_nb(int fd){
    //first intialise the errno as 0
    errno=0;
    //dont know what it does 
    //fcntl :: file control 
    //fcntl(int fd, int cmd, . . . )
    ///cmd :: F_GETFL =>Get File Status Flags.
    // third argument is not used so 0 here
    int flags=fcntl(fd, F_GETFL,0);

    if(errno){
        die("errno in fcrnl()");
        return ;
    }

    flags|=O_NONBLOCK;
    //reset the errno for next operation
    errno=0;
    //is commonly used for manipulating file descriptor flags, locking, and other control operations on files and devices.
    (void)fcntl(fd, F_SETFL,flags);
    if(errno){
        die("fcntl error ");
    }

}

static void buf_append(std::vector<uint8_t> &buf,  const uint8_t *data, size_t len){
    buf.insert(buf.end(), data, data+len);
}

static void buf_consume(std::vector<uint8_t> &buf , size_t n){
    buf.erase(buf.begin(), buf.begin()+n);
}

static Conn* handle_accept(int fd){
    struct sockaddr_in client_addr={};
    socklen_t addrlen =sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr*)&client_addr, &addrlen);

    if(connfd<0){
        msg_errno("error in accepting connection from the client");
        return nullptr;
    }

    uint32_t ip = client_addr.sin_addr.s_addr;
    fprintf(stderr, "new Client from %u.%u.%u.%u:%u\n", ip&255, (ip>>8)&255,(ip>>16)&255,(ip>>32)&255,ntohs(client_addr.sin_port));

    fd_set_nb(connfd);

    Conn *conn =new Conn();
    conn->fd=connfd;
    conn->want_read=true;
    return conn;
}


static bool try_one_request(Conn *conn){
    if(conn->incoming.size()<4){
        return false;
    }

    uint32_t len =0;

    memcpy(&len, conn->incoming.data(),4 );
    if(len>k_msg_max){
        msg("message lenght :: too long");
        conn->want_close=true;
        return false;
    }


    if(4+len>conn->incoming.size()){
        return false;
    }

    const uint8_t *request=&conn->incoming[4];

    printf("client says : len:%d , data : %.*s \n", len, len<100?len:100, request);
    buf_append(conn->outgoing, (const uint8_t*)&len, 4);
    buf_append(conn->outgoing, request, len);

    buf_consume(conn->incoming, 4+len);
    return true;
}

static void handle_write(Conn *conn){
    assert(conn->outgoing.size()>0);
    ssize_t rv=write(conn->fd, &conn->outgoing[0],conn->outgoing.size() );

    if (rv<0 && errno==EAGAIN){
        //soclet is not ready yet
        return;
    }

    if(rv<0){
        msg_errno("write() :: error ");
        conn->want_close=true;
        return;
    }
    buf_consume(conn->outgoing, (size_t)rv);

    if(conn->outgoing.size()==0){
        conn->want_read=true;
        conn->want_write=false;
    }
}


static void handle_read(Conn *conn){
    uint8_t buff[64*1024];

    ssize_t  rv=read(conn->fd, buff, sizeof(buff));
    if(rv<0 && errno==EAGAIN){
        return ;
    }

    if(rv<0){
        msg_errno("read() :: error ");
        conn->want_close=true;
        return;
    }

    if(rv==0){
        if(conn->incoming.size()==0){
            msg("Connection is closed");
        }
        else{
            msg("Unexpected EOF");
        }
        conn->want_close=true;
        return;
    }
    buf_append(conn->incoming, buff,(size_t)rv);

    while(try_one_request(conn)){}

    if(conn->outgoing.size()>0){
        conn->want_read=false;
        conn->want_write=true;

        return handle_write(conn);
    }
}


