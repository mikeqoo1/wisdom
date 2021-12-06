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
#define BUFFER_LEN 1024
#define NAME_LEN 20       //User Name 長度
#define MAX_CLIENT_NUM 32 //最大連線數

struct User_Client //紀錄使用者資訊
{
    int valid;               //to judge whether this user is online
    int fd_id;               //user ID number
    int socket;              //socket to this user
    char name[NAME_LEN + 1]; //name of the user
};

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
    int servRun();
    void *chat(User_Client *data);
    void handle_recv(void *data);
    void *handle_send(User_Client *data); //send message

private:
    int sockfd;
    int current_client_num;
    struct sockaddr_in serverIP;
    User_Client client[MAX_CLIENT_NUM] = {0};
    queue<string> message_q[MAX_CLIENT_NUM]; //message buffer
    std::mutex my_lock;
    std::mutex my_lock_client[MAX_CLIENT_NUM];      //給client用的鎖
    std::condition_variable notify[MAX_CLIENT_NUM]; //條件變數
    std::thread my_chat_thread[MAX_CLIENT_NUM];
    std::thread my_send_thread[MAX_CLIENT_NUM];
};

//https://shengyu7697.github.io/cpp11-how/
//條件變數用來通知各個thread https://baike.baidu.hk/item/pthread_cond_signal/564029

#endif /* SERVER_HPP */