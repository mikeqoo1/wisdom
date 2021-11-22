#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <cassert>
#include <sys/types.h>
#include <netinet/in.h> //htons
#include <string.h>     // memset
#include <arpa/inet.h>  // inet_addr
using namespace std;

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(6666);
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int ret = connect(sockfd, (struct sockaddr *)&servAddr, sizeof(servAddr));
    assert(ret == 0);

    while (1)
    {
        char sendbuffer[1024];
        memset(sendbuffer, 0, sizeof(sendbuffer));
        // int ret = recv(sockfd,buffer,sizeof(buffer),0);
        cout << "輸入訊息:" << endl;
        cin >> sendbuffer;
        send(sockfd, sendbuffer, sizeof(sendbuffer), 0);
        char recvbuf[1024];
        int ret = recv(sockfd, recvbuf, sizeof(recvbuf), 0);
        if (ret)
        {
            cout << "收到的訊息:" << recvbuf << endl;
        }
        else if (ret == 0)
        {
            cout << "斷線" << endl;
            close(sockfd);
            break;
        }
    }
}