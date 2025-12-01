
#ifndef REUSE_H
#define REUSE_H


#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <vector>

static void msg(const char *msg);
static void msg_errno(const char* msg);
static void die(const char* msg);


const size_t k_msg_max = 4096;
static void fd_set_nb(int fd);

struct Conn{
    int fd=-1;

    bool want_read=false;
    bool want_write=false;
    bool want_close=false;

    std::vector<uint8_t> incoming;
    std::vector<uint8_t> outgoing;
};

static void buf_append(std::vector<uint8_t> &buf,  const uint8_t *data, size_t len);
static void buf_consume(std::vector<uint8_t> &buf , size_t n);
static Conn* handle_accept(int fd);
static bool try_one_request(Conn *conn);
static void handle_write(Conn *conn);
static void handle_read(Conn *conn);

#endif //REUSE_H