#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <iostream>

using namespace std;

class Server
{
public:
    Server(int port)
    {
        serverIP.sin_family = AF_INET;
        serverIP.sin_port = htons(port);
        serverIP.sin_addr.s_addr = htonl(INADDR_ANY);
        this->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    }
    ~Server() {}

public:
    int serverBind();
    //maxbacklog指定監聽佇列大小，當有連線請求到來會進入此監聽佇列，連線請求被 accept() 後會離開監聽佇列，當佇列滿時，新的連線請求會返回錯誤
    int serverListen(int maxbacklog);
    void serverAccept();
    void thread_func(int connfd);
    void servRun();

private:
    int sockfd;
    int clientid;
    int connfdlist[10];
    struct sockaddr_in serverIP;
};

#endif /* SERVER_HPP */