#include <cstring>
#include <fstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "ctcp.h"
#include "timestamp.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: ./client ip port\n");
        printf("example: ./client 127.0.0.1 50001\n\n");
        return -1;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("socket() failed.\n");
        return -1;
    }

    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(atoi(argv[2]));

    if (connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connect(%s:%s) failed.\n", argv[1], argv[2]);
        return -1;
    }
    printf("connect ok.\n");

    printf("%s\n", Timestamp::now().tostring().c_str());
    char buf[1024];
    for (int i = 0; i < 1000000; ++i)
    {
        memset(buf, 0, sizeof(buf));
        // printf("please input:");
        // scanf("%s", buf);
        sprintf(buf, "this is the %d message!", i);

        char tmpbuf[1024];
        memset(tmpbuf, 0, sizeof(tmpbuf));
        int len = strlen(buf);
        memcpy(tmpbuf, &len, 4);
        memcpy(tmpbuf + 4, buf, len);

        // 这里不能使用strlen(tmpbuf)，因为前4byte可能出现代表\0的值
        if (send(sockfd, tmpbuf, len + 4, 0) < 0)
        {
            printf("write() failed.\n");
            close(sockfd);
            return -1;
        }
        // }
        // for (int i = 0; i < 3; ++i){
        // int len;
        recv(sockfd, &len, 4, 0);

        memset(buf, 0, sizeof(buf));
        recv(sockfd, buf, len, 0);
        // printf("recv: %s\n", buf);
    }
    printf("%s\n", Timestamp::now().tostring().c_str());
    return 0;
}