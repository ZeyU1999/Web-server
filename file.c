#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"

//删除文件
void delete_file(char* file_name){
  if(remove(file_name) == 0){
    puts("delete seccess!");
    return;
  }
  puts("delete error");
}

//判断操作类型
int judge_action(char* action,char* file_name){
  if(action == NULL){
    return 0;
  }
  if(strcmp(action,"Delete") == 0){
    //删除方法
    delete_file(file_name);
    printf(">>>>>delete success!\n");

    return 1;
  }
  return 0;
}
