#include <unistd.h>
#include <errno.h>
#include <time.h>
#include "sslconnect.h"
#ifndef _TOOLS_H_ 
#define _TOOLS_H_
#define SOCKET_ERROR -1

extern int errno;
int ret;
char const *send_data;

char* currentTime() {
  static char time_str[100];
  time_t time1;
  struct tm *tmp_ptr = NULL;
  time(&time1);
  tmp_ptr = localtime(&time1);
  sprintf(time_str, "%d-%02d-%02d %02d:%02d:%02d ",
          tmp_ptr->tm_year + 1900, tmp_ptr->tm_mon + 1, tmp_ptr->tm_mday,
          tmp_ptr->tm_hour, tmp_ptr->tm_min, tmp_ptr->tm_sec);
  return time_str;
}

int getResponse() {
  FILE* file = fopen("email.log", "a");
  memset(buffer, 0, sizeof(buffer));
  ret = SSL_read(ssl, buffer, 1024);
  fprintf(file, "%s %s - recv: \n%s\n", currentTime(), __func__, buffer);
  if(ret == SOCKET_ERROR) {
    perror("recv");
    return -1;
  }
  buffer[ret]='\0';
  if(*buffer == '5' || *buffer == '-') {
    printf("%s\n", buffer);
    return -1;
  }
  fclose(file);
//  printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n%s\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n", buffer);
  return 0;
}

int login(char* username,char* password) {
  FILE* file = fopen("email.log", "a");
  char ch[100];
  if(username == "" || password == "") {
      return -1;
  }

  send_data = "HELO 163.com\r\n";
  ret = SSL_write(ssl, send_data, strlen(send_data));
  fprintf(file, "%s %s - send HELO: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send HELO");
    return -1;
  }
  if(getResponse() < 0) {
    return -1;
  }

  send_data = "AUTH LOGIN\r\n";
  ret = SSL_write(ssl, send_data, strlen(send_data));
  fprintf(file, "%s %s - send AUTH: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send AUTH");
    return -1;
  }
  if(getResponse() < 0) {
    return -1;
  }

  sprintf(ch, "%s\r\n", username);
  ret = SSL_write(ssl, (char *)ch, strlen(ch));
  fprintf(file, "%s %s - send username: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send username");
    return -1;
  }

  if(getResponse() < 0){
    return -1;
  }

  sprintf(ch,"%s\r\n",password);
  ret = SSL_write(ssl, (char *)ch, strlen(ch));
  fprintf(file, "%s %s - send password: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send password");
    return -1;
  }

  if(getResponse() < 0){
    return -1;
  }
  return 0;
}

void watch_help() {
	printf("--inbox          Check inbox\n");
	printf("-h               Help\n");
	printf("--setuser        Set username and password\n");
	printf("--send <target>  Send email to <target>\n");
}

int getNamePasswd(char name[], char passwd[]) {
  FILE* fp = fopen("email.conf","r");
  if (fp != NULL) {
      //配置文件存在,从配置文件中读取base64加密过的用户名密码
      fscanf(fp, "%s\n%s", name, passwd);
  } else {
      //配置文件不存在，进入配置模块
      printf("Error: No username or password. \"--setuser\" to set username and password\n");
      return -1;
  }
  return 0;
}

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

/* 转换算子 */   
static char find_pos(char ch){   
    char *ptr = (char*)strrchr(base, ch);//the last position (the only) in base[]   
    return (ptr - base);   
}   
  
/* Base64 解码 */   
char *base64_decode(const char *data){
  unsigned long data_len = strlen(data);
  unsigned long ret_len = (data_len / 4) * 3;
    int equal_count = 0;   
    char *ret = NULL;   
    char *f = NULL;   
    int tmp = 0;   
    int temp = 0;  
    int prepare = 0;   
    int i = 0;   
    if (*(data + data_len - 1) == '='){   
        equal_count += 1;   
    }   
    if (*(data + data_len - 2) == '='){   
        equal_count += 1;   
    }   
    if (*(data + data_len - 3) == '='){  
        equal_count += 1;   
    }   
    switch (equal_count) {
      case 0:
      case 1:
          ret_len += 4;//3 + 1 [1 for NULL]
          break;
      case 2:
          ret_len += 3;//Ceil((6*2)/8)+1
          break;
      case 3:
          ret_len += 2;//Ceil((6*1)/8)+1
          break;
      default:
        break;
    }   
    ret = (char *)malloc(ret_len);   
    if (ret == NULL){   
        printf("No enough memory.\n");   
        exit(0);   
    }   
    memset(ret, 0, ret_len);   
    f = ret;   
    while (tmp < (data_len - equal_count)){   
        temp = 0;   
        prepare = 0;   
        while (temp < 4){   
            if (tmp >= (data_len - equal_count)){   
                break;   
            }   
            prepare = (prepare << 6) | (find_pos(data[tmp]));   
            temp++;   
            tmp++;   
        }   
        prepare = prepare << ((4-temp) * 6);   
        for (i=0; i<3 ;i++ ){   
            if (i == temp){   
                break;   
            }   
            *f = (char)((prepare>>((2-i)*8)) & 0xFF);   
            f++;   
        }   
    }   
    *f = '\0';   
    return ret;   
}

/* base64 编码 */
char *base64_encode(const char* data){
  int data_len = strlen(data);
  int prepare = 0;
  int ret_len;
  int temp = 0;
  char *ret = NULL;
  char *f = NULL;
  int tmp = 0;
  char changed[4];
  int i = 0;
  ret_len = data_len / 3;
  temp = data_len % 3;
  if (temp > 0){
    ret_len += 1;
  }
  ret_len = ret_len*4 + 1;
  ret = (char *)malloc(ret_len);

  if ( ret == NULL){
    printf("No enough memory.\n");
    exit(0);
  }
  memset(ret, 0, ret_len);
  f = ret;
  while (tmp < data_len){
    temp = 0;
    prepare = 0;
    memset(changed, '\0', 4);
    while (temp < 3){
      if (tmp >= data_len){
        break;
      }
      prepare = ((prepare << 8) | (data[tmp] & 0xFF));
      tmp++;
      temp++;
    }
    prepare = (prepare<<((3-temp)*8));
    for (i = 0; i < 4 ;i++ ){
      if (temp < i){
        changed[i] = 0x40;
      }else{
        changed[i] = (prepare>>((3-i)*6)) & 0x3F;
      }
      *f = base[changed[i]];
      f++;
    }
  }
  *f = '\0';

  return ret;

}

//设置用户信息
void setUser() {
  printf("Username:");
  char name[100],*passwd;
  scanf("%s",name);
  getchar();
  passwd = getpass("Password:");
  char* base64_name = base64_encode(name);
  char* base64_passwd = base64_encode(passwd);
  FILE* fp = fopen("email.conf","w");
  fprintf(fp, "%s\n%s",base64_name,base64_passwd);
  fclose(fp);
}

#endif
