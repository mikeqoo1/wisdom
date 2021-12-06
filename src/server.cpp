#include "server.hpp"

//子執行緒入口func 負責發msg
void *Server::handle_send(User_Client *data)
{
    struct User_Client *pipe = (struct User_Client *)data;
    while (1)
    {
        //wait until new message receive
        while (message_q[pipe->fd_id].empty())
        {
            std::unique_lock<std::mutex> lock(my_lock_client[pipe->fd_id]);
            notify[pipe->fd_id].wait(lock);
        }
        //if message queue isn't full, send message
        while (!message_q[pipe->fd_id].empty())
        {
            //get the first message from the queue
            string message_buffer = message_q[pipe->fd_id].front();
            int n = message_buffer.length();
            //calculate one transfer length
            int trans_len = BUFFER_LEN > n ? n : BUFFER_LEN;
            //send the message
            while (n > 0)
            {
                int len = send(pipe->socket, message_buffer.c_str(), trans_len, 0);
                if (len < 0)
                {
                    perror("send");
                    return NULL;
                }
                n -= len;
                message_buffer.erase(0, len); //delete data that has been transported
                trans_len = BUFFER_LEN > n ? n : BUFFER_LEN;
            }
            //delete the message that has been sent
            message_buffer.clear();
            message_q[pipe->fd_id].pop();
        }
    }
    return NULL;
}

//子執行緒入口func 負責收msg
void Server::handle_recv(void *data)
{
    struct User_Client *pipe = (struct User_Client *)data;
    // message buffer
    string message_buffer;
    int message_len = 0;

    // one transfer buffer
    char buffer[BUFFER_LEN + 1];
    int buffer_len = 0;

    // receive
    while ((buffer_len = recv(pipe->socket, buffer, BUFFER_LEN, 0)) > 0)
    {
        //to find '\n' as the end of the message
        for (int i = 0; i < buffer_len; i++)
        {
            //the start of a new message
            if (message_len == 0)
            {
                char temp[100];
                sprintf(temp, "%s:", pipe->name);
                message_buffer = temp;
                message_len = message_buffer.length();
            }

            message_buffer += buffer[i];
            message_len++;

            if (buffer[i] == '\n')
            {
                //send to every client
                for (int j = 0; j < MAX_CLIENT_NUM; j++)
                {
                    if (client[j].valid && client[j].socket != pipe->socket)
                    {
                        message_q[j].push(message_buffer);
                        notify[j].notify_one();
                    }
                }
                //new message start
                message_len = 0;
                message_buffer.clear();
            }
        }
        //clear buffer
        buffer_len = 0;
        memset(buffer, 0, sizeof(buffer));
    }
    return;
}

void *Server::chat(User_Client *data)
{
    struct User_Client *pipe = (struct User_Client *)data;

    //printf hello message
    char hello[100];
    sprintf(hello, "Hello %s, Welcome to join the chat room. Online User Number: %d\n", pipe->name, current_client_num);
    message_q[pipe->fd_id].push(hello);
    notify[pipe->fd_id].notify_one();

    memset(hello, 0, sizeof(hello));
    sprintf(hello, "New User %s join in! Online User Number: %d\n", pipe->name, current_client_num);
    //send messages to other users
    for (int j = 0; j < MAX_CLIENT_NUM; j++)
    {
        if (client[j].valid && client[j].socket != pipe->socket)
        {
            message_q[j].push(hello);
            notify[j].notify_one();
        }
    }

    //create a new thread to handle send messages for this socket
    cout << "fd_id ==" << pipe->fd_id << endl;
    my_send_thread[pipe->fd_id] = std::thread(&Server::handle_send, this, pipe);
    my_send_thread[pipe->fd_id].detach();

    //receive message
    handle_recv(data);

    //because the recv() function is blocking, so when handle_recv() return, it means this user is offline
    my_lock.lock();
    pipe->valid = 0;
    current_client_num--;
    my_lock.unlock();
    //printf bye message
    printf("%s left the chat room. Online Person Number: %d\n", pipe->name, current_client_num);
    char bye[100];
    sprintf(bye, "%s left the chat room. Online Person Number: %d\n", pipe->name, current_client_num);
    //send offline message to other clients
    for (int j = 0; j < MAX_CLIENT_NUM; j++)
    {
        if (client[j].valid && client[j].socket != pipe->socket)
        {
            message_q[j].push(bye);
            notify[j].notify_one();
        }
    }


    return NULL;
}

int Server::servRun()
{
    current_client_num = 0;
    if (bind(sockfd, (struct sockaddr *)&serverIP, sizeof(serverIP)))
    {
        cout << "bind error" << endl;
        return -1;
    }
    //最多 32 個客戶端
    if (listen(sockfd, MAX_CLIENT_NUM + 1))
    {
        cout << "listen error" << endl;
        return -1;
    }
    cout << "Server start successfully!" << endl;
    cout << "You can join the chat room by connecting to 127.0.0.1:6666" << endl
         << endl;

    //waiting for new client to join in
    while (1)
    {
        //create a new connect
        struct sockaddr_in clitAddr;
        socklen_t clitLen = sizeof(clitAddr);
        int connfd = accept(sockfd, (struct sockaddr *)&clitAddr, &clitLen);

        if (connfd == -1)
        {
            cout << "accept error!" << endl;
            return -1;
        }

        //check whether is full or not
        if (current_client_num >= MAX_CLIENT_NUM)
        {
            if (send(connfd, "ERROR", strlen("ERROR"), 0) < 0)
                cout << "send" << endl;
            shutdown(connfd, 2);
            continue;
        }
        else
        {
            if (send(connfd, "OK", strlen("OK"), 0) < 0)
                cout << "send" << endl;
        }

        //get client's name
        char name[NAME_LEN + 1] = {0};
        ssize_t state = recv(connfd, name, NAME_LEN, 0);
        if (state < 0)
        {
            cout << "recv" << endl;
            shutdown(connfd, 2);
            continue;
        }
        //new user do not input a name but leave directly
        else if (state == 0)
        {
            shutdown(connfd, 2);
            continue;
        }

        //update client array, create new thread
        for (int i = 0; i < MAX_CLIENT_NUM; i++)
        {
            //find the first unused client
            if (!client[i].valid)
            {
                my_lock.lock();
                //設定名稱跟其他資訊
                memset(client[i].name, 0, sizeof(client[i].name));
                strcpy(client[i].name, name);
                client[i].valid = 1;
                client[i].fd_id = i;
                client[i].socket = connfd;
                //客戶ID
                current_client_num++;
                my_lock.unlock();

                //create new receive thread for new client
                /*http://programmingpaul.blogspot.com/2013/08/c-thread-function.html*/
                my_chat_thread[i] = std::thread(&Server::chat, this, &client[i]);
                my_chat_thread[i].detach();

                printf("%s join in the chat room. Online User Number: %d\n", client[i].name, current_client_num);
                break;
            }
        }
    }

    //close socket
    for (int i = 0; i < MAX_CLIENT_NUM; i++)
        if (client[i].valid)
            shutdown(client[i].socket, 2);
    shutdown(sockfd, 2);
    return 0;
}