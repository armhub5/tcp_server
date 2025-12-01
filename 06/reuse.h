
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

  void msg(const char *msg);
  void msg_errno(const char* msg);
  void die(const char* msg);


  const size_t k_msg_max = 4096;
    void fd_set_nb(int fd);

  struct Conn{
      int fd=-1;

      bool want_read=false;
      bool want_write=false;
      bool want_close=false;

      std::vector<uint8_t> incoming;
      std::vector<uint8_t> outgoing;
  };

  void buf_append(std::vector<uint8_t> &buf,  const uint8_t *data, size_t len);
  void buf_consume(std::vector<uint8_t> &buf , size_t n);
  Conn* handle_accept(int fd);
  bool try_one_request(Conn *conn);
  void handle_write(Conn *conn);
  void handle_read(Conn *conn);
  
  const size_t cl_k_max_msg = 32<<20;
  
  int32_t read_full(int fd, uint8_t *buf, size_t n);
  int32_t write_all(int fd ,const uint8_t * buf, size_t n);
  int32_t send_req(int fd, const uint8_t *text, size_t len);
  int32_t read_res(int fd);
#endif //REUSE_H