#ifndef NETWORK_H
#define NETWORK_H

#define BACKLOG 5
#define MAX_MSG_LEN 1024
#define MSG_LENTH 1024
#define SHORT_LENTH 100

//用户信息的定义
typedef struct{ 
  char info[SHORT_LENTH]; //用户信息
  char opened_file[SHORT_LENTH]; //访问过的文件名
}User;

//链表节点
typedef struct Node {
  User user ;
  struct Node *next;
}Node, *LinkList;

void client_info(struct sockaddr_in client);
char* find_addr(char* buf);
char* find_action(char* file_path);
char* find_filename(char* file_path);
int reorientation(char* file_name, int connectfd);
void rcv_msg(int connectfd);
void send_msg(int connectfd, char* state, char* file);
int srv_biz(int connectfd);
int TCP_init(char* IP, int PORT);
char* find_addr_a(char* buf);
void file_path_init(char* file_path);
int security_identify(int connectfd ,int source_fd);
char* getchar_security();
char* getchar_userinfo();
void initList(LinkList *L);
void insertList(LinkList* head,char* info,char* opened_file);
void printfList(LinkList head);
void get_time();
void wirte_log();

#endif