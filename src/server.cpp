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
        cout << "Thread is " << mythread.get_id() << endl;
        //與該執行緒分離，一旦該執行緒執行完後它所分配的資源會被釋放
        mythread.detach();
    }
}

//子執行緒入口func 負責收發msg
void Server::thread_func(int connfd)
{
    clientid++;
    cout << "clientid=" << clientid << endl;
    int onlyID = clientid;
    connfdlist[onlyID] = connfd;
    //read msg & send msg
    while (1)
    {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int ret = recv(connfd, buffer, sizeof(buffer), 0);
        if (ret)
        {
            //商業邏輯可以塞這邊
            cout << "客戶[" << onlyID << "]送的Msg:" << buffer << endl;
            char sendBuf[2048];
            memset(sendBuf, 0, sizeof(sendBuf));
            // cout << "Please Input:" << endl;
            // cin >> sendBuf;
            // cout << "Server回應給[" << onlyID << "]的Msg:" << sendBuf << endl;
            sprintf(sendBuf, "%s%d%s%s", "ID ", onlyID, " send msg is ", buffer);
            send(connfd, sendBuf, sizeof(sendBuf), 0);
            //廣播Msg
            // for (int i = 0; i < 10; i++)
            // {
            //     send(connfdlist[i], sendBuf, sizeof(sendBuf), 0);
            // }
        }
        else if (ret == 0)
        {
            cout << "客戶[" << onlyID << "]斷線" << endl; //斷線
            close(connfd);
            break;
        }
    }
    close(sockfd);
}

void Server::servRun()
{
    clientid = 0;
    serverBind();
    serverListen(5);
    serverAccept();
}