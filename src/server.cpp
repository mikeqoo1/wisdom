#include "server.hpp"

int Server::serverBind()
{
    int ret = bind(sockfd, (struct sockaddr *)&serverIP, sizeof(serverIP));
    assert(ret == 0);
    return 0;
}

int Server::serverListen(int maxbacklog)
{
    int ret = listen(sockfd, maxbacklog);
    assert(ret == 0);
    return 0;
}

void Server::serverAccept()
{
    struct sockaddr_in clitAddr;
    socklen_t clitLen = sizeof(clitAddr);

    while (1)
    { //main thread 持續等待新的連線
        int connfd = accept(sockfd, (struct sockaddr *)&clitAddr, &clitLen);
        std::thread mythread(std::mem_fn(&Server::thread_func), this, connfd); //這裡需要特别注意了解
        mythread.detach();
    }
}

//子執行緒入口func 負責收發msg
void Server::thread_func(int connfd)
{
    //read msg & send msg
    while (1)
    {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int ret = recv(connfd, buffer, sizeof(buffer), 0);
        if (ret)
        {
            cout << "ret clit:" << buffer << endl;
            char sendBuf[1024];
            memset(sendBuf, 0, sizeof(sendBuf));
            cout << "Please Input:" << endl;
            cin >> sendBuf;
            send(connfd, sendBuf, sizeof(sendBuf), 0);
        }
        else if (ret == 0)
        {
            cout << "link failure!" << endl; //斷線
            close(connfd);
            break;
        }
    }
    close(sockfd);
}

void Server::servRun()
{
    serverBind();
    serverListen(5);
    serverAccept();
}