#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h>
#include "network.h"
#include <time.h>

extern char buf[MAX_MSG_LEN];
extern char state_200[MSG_LENTH];
extern char state_404[MSG_LENTH];
extern char state_301[MSG_LENTH];
extern char state_401[MSG_LENTH];
extern char file_path[MAX_MSG_LEN];
extern char file[MSG_LENTH];
extern char real_path[MSG_LENTH];
extern char extracted_str[50];

extern char proctect_file[MAX_MSG_LEN];
extern char identify_msg[MAX_MSG_LEN];
extern char real_buf[SHORT_LENTH];
extern char user_info[SHORT_LENTH];
extern char current_time[20];
extern char log_to_write[MAX_MSG_LEN];

extern LinkList head;

//初始化file_path
void file_path_init(char* file_path){
  bzero(file_path,sizeof(file_path));
  strcpy(file_path,".");
}

//获取目的地址(空格前和空格后)
char* find_addr(char* buf){
  //分离路径
  char* first_line_end = strchr(buf, '\n');  // 找到第一个换行符的位置
  if (first_line_end != NULL) {
      *first_line_end = '\0';  // 将第一个换行符替换为字符串结束符

      char* path_start = strstr(buf, " ");  // 查找第一个空格的位置
      char* path_end = strrchr(buf, ' ');   // 查找最后一个空格的位置

      if (path_start != NULL && path_end != NULL && path_start < path_end) {
          // 提取
          int path_length = path_end - path_start - 1;
          char path[100];  // 假设路径不超过 100 个字符
          strncpy(path, path_start + 1, path_length);
          path[path_length] = '\0';

          bzero(real_path,sizeof(real_path));
          strcpy(real_path,path);
          printf("Path: %s\n", real_path);
          return real_path;
      } else {
          printf("Invalid request line\n");
      }
  }
}

//获取带参数情况下的路径地址（空格后？前）
char* find_addr_a(char* buf){
    // 查找'?'字符的位置
    char* question_mark_pos = strchr(buf, '?');

    if (question_mark_pos != NULL) {
        // 计算'?'之前的字符个数
        size_t length = question_mark_pos - buf;
        // 为新的字符串分配内存（需要在调用函数后手动释放内存）
        char* result = (char*)malloc(length + 1);
        if (result != NULL) {
            // 复制'?'之前的字符到新的字符串中
            strncpy(result, buf, length);
            result[length] = '\0'; // 添加字符串结束符
            return result;
        } else {
            printf("Memory allocation failed.\n");
            return NULL;
        }
    } else {
        printf("No '?' character found in the input string.\n");
        return NULL;
    }
}

//获取方法类型（/后和？前）
char* find_action(char* file_path){
  const char* question_mark = strchr(file_path, '?');
    if (question_mark != NULL) {
      const char* slash_before_question = NULL;
      // 从question_mark位置向前搜索第一个'/'
      for (const char* ptr = question_mark - 1; ptr >= file_path; --ptr) {
        if (*ptr == '/') {
          slash_before_question = ptr;
           break;
        }
      }
      // 复制子串
      if (slash_before_question != NULL) {
        size_t length = question_mark - slash_before_question - 1;
        char* result = (char*)malloc(length + 1); // 为结果字符串分配内存
        if (result != NULL) {
          strncpy(result, slash_before_question + 1, length);
          result[length] = '\0'; // 添加字符串结束符
          return result;
        } else {
          printf("Memory allocation failed.\n");
          return NULL;
        }
      }
    }
  return NULL; // 如果没有找到合适的子串，返回NULL
}

//获取文件名字(=到字符串末尾)
char* find_filename(char* file_path) {
  char* start_str = strchr(file_path, '=');

  if (start_str != NULL) {
    int start_index = start_str - file_path + 1;
    int length = strlen(file_path) - start_index;

    char* extracted_str = malloc((length + 1) * sizeof(char));
    strncpy(extracted_str, file_path + start_index, length);
    extracted_str[length] = '\0';

    return extracted_str;
    }
  return NULL;
}

//重定向
int reorientation(char* file_name,int connectfd){
  //判断是否可以重定向
  if( strcmp(file_name,"./www/site1/html/123.html") == 0){
    puts(">>>>>re begin!");
    int re_fd = open("./www/site1/html/321.html",O_RDONLY);
    if (re_fd == -1){
      puts("reorientation file open error!");
      return 0;
    }
    //读文件
    bzero(file,sizeof(file));
    read(re_fd, file, sizeof(file));
    //发送响应
    bzero(buf,sizeof(buf));
    strcpy(buf,state_301);
    strcat(buf,file);
    //总长度
    write(connectfd, buf, strlen(buf));
    return 1;
  }
  return 0;
}

//接收buf中消息
void rcv_msg(int connectfd){
  //接收请求
  bzero(buf,sizeof(buf));
  ssize_t size_read = read(connectfd, buf, sizeof(buf));
  printf("receive:%s",buf);
}

// 发送buf消息（带头）
void send_msg(int connectfd,char* state,char* file){
  //发送响应
  bzero(buf,sizeof(buf));
  strcpy(buf,state);
  if( file != NULL){
    strcat(buf,file);
  }
  printf("send:%s",buf);
  //总长度
  write(connectfd, buf, strlen(buf));
}

// 收发函数
int srv_biz(int connectfd) {
  // 接收消息
  rcv_msg(connectfd);
  // 寻找目的路径
  bzero(real_buf,sizeof(real_buf));
  strcpy(real_buf,buf);
  char* real_path = find_addr(buf);
  // 拼接路径，前面加.
  strcat(file_path,real_path);
  // 打开文件
  printf("final_file_path:%s\n",file_path);
  int source_fd = open(file_path, O_RDONLY);
  // 打开失败
  if (source_fd == -1) {
      //puts(">>>file open fail");
      //把参数去掉
      char* addr_a = find_addr_a(file_path);
      int source_fd_a = open(addr_a, O_RDONLY);
      if( source_fd_a != -1){
        bzero(file,sizeof(file));
        read(source_fd_a,file,sizeof(file));
        send_msg(connectfd,state_200,file);
        file_path_init(file_path);
        return 1;
      }
      //重定向
      if(reorientation(file_path,connectfd) == 1){
        file_path_init(file_path);
        return 1;
      }
      //参数前为动作
      char* action = find_action(file_path);//获取动作
      char* file_name = find_filename(file_path);//获取文件名
      printf("file name:%s\n",file_name);
      //进行操作
      if(judge_action(action,file_name) == 0){
        //不是操作
        printf(">>>>action error!\n");
        write(connectfd, state_404, strlen(state_404));
        file_path_init(file_path);
        return 1;
      }
      //是操作
      puts("file to find!\n");
      write(connectfd, state_200, strlen(state_200));     
      printf("Failed to open source file\n");
      file_path_init(file_path);
      return 1;
  }
  // 安全认证
  if(security_identify(connectfd,source_fd) == 0){
    //puts(">>>>>request for user identify!");
    return 1;
  }
  // 打开成功，记录文件
  if( strlen(user_info)!= 0 ){
    insertList( &head,user_info,file_path );
    printf(">>>>>>>>point_out:%p\n",head);
  }
  // 正常读写文件
  bzero(file,sizeof(file));
  read(source_fd, file, sizeof(file));
  if( strcmp(file_path,"./www/site1/html/log.html") == 0){
    //读的是日志文件
    strcat(file,"</body>\n</html>");
  }
  send_msg(connectfd,state_200,file);
  file_path_init(file_path);
  return 1;
}

//建立连接
int TCP_init(char* IP,int PORT ){
  //监听套接字和连接套接字
	int 	listenfd, connectfd;
  //服务器以及客户端的地址信息
	struct sockaddr_in	server, client;		
	int	sin_size;			
  //创建套接字
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0))==-1)	
	{
    //创建失败
		perror("Create socket failed.");		//perror是系统函数
		exit(-1);	
  }
  //可立即重用
  int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));	
  
  //绑定套接字和端口
  bzero(&server, sizeof(server));						
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
  
  //绑定成传入的ip地址
  //将字符串形式的IPv4地址转换为二进制形式？
  inet_pton(AF_INET, IP, &(server.sin_addr));
  
  //把server里的地址信息绑定到监听套接字上
	if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr))==-1) {	
		perror("Bind error.");
		exit(-1);
	}

  //开始监听
	if (listen(listenfd, BACKLOG) == -1) {
		perror("listen error.");
		exit(-1);
	}

  //输出初始化信息
  printf("[srv] server[%s:%d] is initialiazing!\n",IP,PORT);
  printf("[srv] Server has initialized!\n");
  return listenfd;
}

//输出客户端信息
void client_info(struct sockaddr_in client){
  // 受理客户端请求之后输出
  char* client_addr = inet_ntoa(client.sin_addr);
  int client_port = ntohs(client.sin_port);
  printf("[srv] client[%s:%d] is accepted\n", client_addr, client_port);
}

//获取安全认证中的用户名和密码到user_info
char* getchar_userinfo(){
  bzero(user_info,sizeof(user_info));
  const char* basic_prefix = ": Basic ";
  const size_t basic_prefix_len = strlen(basic_prefix);

    // 检查Authorization值是否以": Basic "开头
  if (strncmp(identify_msg, basic_prefix, basic_prefix_len) != 0) {
    return NULL; // 如果不是以": Basic "开头，则返回NULL
  }

    // 提取Basic后的所有字符（不包含换行符）
  const char* credentials = identify_msg + basic_prefix_len;
  size_t credentials_len = strcspn(credentials, "\r\n"); // 查找换行符的位置
  char* extracted_credentials = (char*)malloc(credentials_len + 1);
  strncpy(extracted_credentials, credentials, credentials_len);
  extracted_credentials[credentials_len] = '\0';
  strcpy(user_info,extracted_credentials);
  printf(">>>>>>user info:%s\n",user_info);
  return user_info;
}

//解析安全认证
char* getchar_security(){
    //printf(">>>TEST%s\n",real_buf);
    bzero(identify_msg,sizeof(identify_msg));
    const char* auth_header = "Authorization";
    const size_t auth_header_len = strlen(auth_header);

    // 查找Authorization头部在请求报文中的位置
    const char* auth_line = strstr(real_buf, auth_header);
    if (auth_line == NULL) {
        return NULL; // 如果找不到Authorization头部，则返回NULL
    }

    // 提取Authorization头部信息的值
    const char* auth_value = auth_line + auth_header_len;

    // 寻找值的结束位置（可能是换行符或结尾）
    const char* end_of_auth_value = strchr(auth_value, '\r');
    if (end_of_auth_value == NULL) {
        end_of_auth_value = strchr(auth_value, '\n');
    }

    if (end_of_auth_value == NULL) {
        end_of_auth_value = auth_value + strlen(auth_value);
    }

    // 计算值的长度
    size_t auth_value_len = end_of_auth_value - auth_value;

    // 创建一个新的字符串来保存Authorization头部信息的值，并返回
    char* extracted_value = (char*)malloc(auth_value_len + 1);
    strncpy(extracted_value, auth_value, auth_value_len);
    extracted_value[auth_value_len] = '\0';
    strcpy(identify_msg,extracted_value);
    return identify_msg;
}

//安全认证
int security_identify(int connectfd ,int source_fd){
  //判断文件是否受到保护
  if(strcmp(file_path,proctect_file) == 0){
    if(getchar_security() == NULL){
      //不是认证消息
      send_msg(connectfd,state_401,NULL);
      return 1;
    }else{
      //是认证消息
      //把认证信息存储起来
      getchar_userinfo();
      // 记录访问protect的信息
      puts("!!public test");
      insertList( &head,user_info,file_path );
      //发送受到保护的文件
      bzero(file,sizeof(file));
      read(source_fd, file, sizeof(file));
      send_msg(connectfd,state_200,file);
      file_path_init(file_path);
      return 0;
    }
  }
  return 1;
}

//链表创建
void initList(LinkList *L) {
  //头结点的建立
  *L=(LinkList)malloc(sizeof(Node)); 
  (*L)->next=NULL; 
}

//链表插入(尾插法)
void insertList(LinkList* head,char* info,char* opened_file) {
  // 建立一个节点并赋值
  LinkList p = (LinkList)malloc(sizeof(Node));
  p->next = NULL;
  strcpy(p->user.info, info);
  strcpy(p->user.opened_file, opened_file);
  // 处理头节点为空的情况
  if (*head == NULL) {
      puts(">>>>>>>head is empty!");
      *head = p;
      return;
  }
  // 找到尾部节点
  // LinkList tail = *head;
  // while (tail->next != NULL) {
  //   tail = tail->next;
  // }
  // 将新节点插入到尾部s
  //tail->next = p;
  //获取时间
  get_time();
  //写入文件
  sprintf(log_to_write, "<p>User: %s, File: %s, Time: %s</p>\n", p->user.info, p->user.opened_file, current_time);
  wirte_log();
  bzero(current_time,sizeof(current_time));
  printf(">>>>>>>>point_out:%p\n",head);
} 

//链表打印
void printfList(LinkList head){
  LinkList p = head->next;
  while(p != NULL){
    printf("%s,%s",p->user.info,p->user.opened_file);
    get_time();
    printf("-->\n");
    bzero(current_time,sizeof(current_time));
    p = p->next;
  }
}

//获取时间
void get_time(){
    time_t timep;
    time(&timep);

    struct tm *p;
    p = gmtime(&timep);

    snprintf(current_time, 20, "%d-%d-%d %d:%2d:%2d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, 8 + p->tm_hour, p->tm_min, p->tm_sec);

    printf("%s", current_time);
    return;
}

//写入日志
void wirte_log(){
  // 打开文件 log.html，以追加模式（"a"）写入
  FILE* file = fopen("./www/site1/html/log.html", "a");
  if (file == NULL) {
    printf(">>>>>>>Failed to open log.html.\n");
    return;
  }

  // 将字符串 log_to_write 写入文件
  fprintf(file, "%s", log_to_write);

  // 关闭文件
  fclose(file);

  printf("Content has been written to log.html.\n");
  bzero(log_to_write,sizeof(log_to_write));
  return;
}


