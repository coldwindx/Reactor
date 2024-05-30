#include <cstring>
#include <fstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <signal.h>

#include "address.h"
#include "socket.hpp"
#include "epoll.hpp"
#include "loop.hpp"
#include "tcpserver.hpp"
#include "echoserver.hpp"

EchoServer *server;
void stop(int sig)
{
    // printf("sig=%d\n", sig);
    server->stop();
    delete server;
    printf("echoserver has stoped.\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: ./server ip port\n");
        printf("example: ./server 127.0.0.1 50001\n\n");
        return -1;
    }

    signal(SIGTERM, stop);
    signal(SIGINT, stop);

    server = new EchoServer(argv[1], atoi(argv[2]), 10, 0);
    server->start();

    return 0;
}