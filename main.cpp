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

using namespace std;
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
		die("bind()");
	}

	rv = listen(fd, SOMAXCONN);
	if (rv)
	{
		die("listen()");
	}

	while (true)
	{
		// accept
		struct sockaddr_in client_addr = {};
		socklen_t addrlen = sizeof(client_addr);
		int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
		if (connfd < 0)
		{
			continue; // error
		}
		do_something(connfd);
		close(connfd);
	}
	cout << "hello mother fucker" << endl;
	return 0;
}
