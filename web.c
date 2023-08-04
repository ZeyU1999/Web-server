#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "network.h"
#include "tool.h"
/*
    字符串写入太呆了
    初始化文件
*/

#define BACKLOG 5
#define MAX_MSG_LEN 1024
#define MSG_LENTH 1024
#define SHORT_LENTH 100

char buf[MAX_MSG_LEN];
char real_buf[SHORT_LENTH];
char state_200[MSG_LENTH] = "HTTP/1.1 200 OK\nDate: Tue, 18 Jul 2023 01:40:52 GMT\nContent-Type: text/html; charset=utf-8\n\n";
char state_404[MSG_LENTH] = "HTTP/1.1 404 Not Found\nDate: Tue, 18 Jul 2023 01:40:52 GMT\nContent-Type: text/html; charset=utf-8\n\n";
char state_301[MSG_LENTH] = "HTTP/1.1 301 Moved Permanently\n\n";
char state_401[MSG_LENTH] = "HTTP/1.1 401 Unauthorized\nWWW-Authenticate: Basic\n\n";
char file_path[MAX_MSG_LEN] = ".";
char file[MSG_LENTH]; //服务器读取出的文件
char real_path[MSG_LENTH]; //完整的请求报文
char extracted_str[50];
char user_info[SHORT_LENTH] = {0}; //收到的用户账号和密码
char proctect_file[MAX_MSG_LEN] = "./www/site1/html/protect.html";// 受保护的文件
char identify_msg[MAX_MSG_LEN];
char current_time[20] = {0};
char log_to_write[SHORT_LENTH] = {0};

LinkList head;

int sigint_flag = 0;

int main(int argc, char* argv[]) {
    //安装SIGINT信号处理
    load_sigint();

    //TCP连接初始化
    int connectfd;
    struct sockaddr_in client;
    int sin_size = sizeof(struct sockaddr_in);
    int listenfd = TCP_init(argv[1], atoi(argv[2]));
    
    //创建链表
    initList(&head);

    //建立连接
    while (!sigint_flag) {
        if ((connectfd = accept(listenfd, (struct sockaddr*)&client, &sin_size)) == -1) {
            if (errno == EINTR)
                continue;
            break;
        }
        client_info(client);
        if (srv_biz(connectfd) == 0) {
            close(connectfd);
            close(listenfd);
            return 0;
        }
        close(connectfd);
    }
    close(listenfd);
    printf("[srv] client is closed\n");
    return 0;
}
