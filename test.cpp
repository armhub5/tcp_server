// server.cpp
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

static void die(const char* msg) {
    int err = errno;
    std::fprintf(stderr, "[%d] %s\n", err, msg);
    std::abort();
}

int main() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) die("socket");

    int opt = 1;
    if (::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        die("setsockopt");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // bind to 127.0.0.1

    if (::bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        die("bind");
    if (::listen(fd, 8) < 0)
        die("listen");

    std::puts("Server listening on 127.0.0.1:1234");

    for (;;) {
        sockaddr_in peer{};
        socklen_t plen = sizeof(peer);
        int cfd = ::accept(fd, (sockaddr*)&peer, &plen);
        if (cfd < 0) die("accept");

        char buf[256]{};
        ssize_t n = ::read(cfd, buf, sizeof(buf) - 1);
        if (n < 0) { ::close(cfd); die("read"); }

        const char reply[] = "world";
        if (::write(cfd, reply, sizeof(reply) - 1) < 0) {
            ::close(cfd); die("write");
        }

        ::close(cfd);
    }

    ::close(fd);
    return 0;
}

