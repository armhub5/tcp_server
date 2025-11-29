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

class Reuse{
    public:
        int fd;
        char* buff;
        size_t n;
    
        Reuse(int fd_, char* temp_buff, size_t temp_b){
            
        };
};
