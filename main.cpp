#include <iostream>
#include <sys/socket.h>
#include <cstring>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <assert.h>


using namespace std;
constexpr size_t K_max_msg = 4096;

static void msg(const char *msg)
{
	fprintf(stderr, "%s\n", msg);
}

static void die(const char *msg)
{
	int err = errno;
	fprintf(stderr, "[%d] %s\n", err, msg);
	abort();
}

static int32_t readfull(int fd, char* buff, size_t n){
	while(n>0){
		ssize_t rv=read(fd, buff, n);
		if (rv<=0){
			return -1;
		}

		assert((size_t)rv<=n);
		n-=(size_t)rv;
		buff+=rv;
	}
	return 0;
}

static int32_t write_all(int fd,const char* buf, size_t n){
	while(n>0){
		ssize_t rv = write(fd, buf, n);
		if(rv<=0){
			return -1;
		}
		assert((size_t)rv <=n);
		n-=(size_t)rv;
		buf+=rv;
	}
	return 0;
}

static int32_t one_request(int connfd){
	char rbuff[4+K_max_msg];
	errno=0;

	int32_t err= readfull(connfd, rbuff, 4);
	if(err){
		msg(errno==0 ? "EOF" : "read() error");
		return err;
	}

	uint32_t len_net = 0;
	memcpy(&len_net, rbuff, 4);

	uint32_t len = ntohl(len_net);

	// ... (read the 4-byte length 'len')
	if (len > K_max_msg) {
		msg("too long");
		return -1;
	}
	// FIX: Read 'len' bytes, starting after the 4-byte length header
	err = readfull(connfd, &rbuff[4], len); // Change '4' to 'len'
	if (err) {
		msg("read() :: error");
		return err; // Added return err for consistent error flow
	}
	
	printf("client says :: %.*s \n",len, &rbuff[4]);
	const char reply[]="World";
	// ...
	char wbuff[4+sizeof(reply)];
	len=(uint32_t)strlen(reply);
	memcpy(wbuff, &len, 4); 
	// FIX: Destination must be wbuff[4]
	memcpy(&wbuff[4], reply, len); 

	return write_all(connfd, wbuff, 4+len);
}

static void do_something(int connfd)
{
	char rbuf[64] = {};
	ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
	if (n < 0)
	{
		msg("read() error");
		return;
	}
	printf("client says: %s\n", rbuf);

	char wbuf[] = "world";
	write(connfd, wbuf, strlen(wbuf));
}

// struct sockaddr_in {
//     uint16_t       sin_family; // AF_INET
//     uint16_t       sin_port;   // port in big-endian
//     struct in_addr sin_addr;   // IPv4
// };

// struct in_addr {
//     uint32_t       s_addr;     // IPv4 in big-endian
// };

int main()
{

	// int socket(int domain, int type, int protocol);
	//
	// domain :: AF_NET :: used for IPV4 protocol
	// type :: SOCK_STREAM :: used for the TCP connection , as the TCP receives the data in the form of a stream
	// type :: SOCK_DGRAM :: used for UDP connection, man :: Supports datagrams (connectionless, unreliable messages of a fixed maximum length)
	// man docs mention datagram which stands for the UDP
	// protocol :: Normally only a single protocol exists to support a  particular  socket type  within  a  given  protocol family, in which case protocol can be specified as 0
	// protocol is not much relevant in this example
	// For AF_INET sockets this means that a socket may bind, except when there is an active listening socket bound to the address.

	// The effect of SO_REUSEADDR is important: if it’s not set to 1, a server program cannot bind to the same IP:port it was using after a restart.

	// holds an IPv4:port pair stored as big-endian numbers, converted by htons() and htonl().
	//
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	int val = 1;
	if (fd == -1)
	{
		die("Socket(. . .)");
	}
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(1234);
	addr.sin_addr.s_addr = htonl(0);
	int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));

	if (rv)
	{
		// Check if the specific error is EADDRINUSE
        if (errno == EADDRINUSE) {
            fprintf(stderr, "🚨 WARNING: Port %d is already in use by another process. Please choose a different port.\n", 1234);
            // Optionally, exit gracefully instead of aborting
            return -1; 
        }
		die("bind()");
	}

	rv = listen(fd, SOMAXCONN);
	if (rv)
	{
		die("listen()");
	}

	while (true) {
        // accept
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if (connfd < 0) {
            continue;   // error
        }

        while (true) {
            // here the server only serves one client connection at once
            int32_t err = one_request(connfd);
            if (err) {
                break;
            }
        }
        close(connfd);
    }
	cout << "hello mother fucker" << endl;
	return 0;

}
