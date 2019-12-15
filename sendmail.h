#ifndef _SENDMAIL_H_ 
#define _SENDMAIL_H_
#include "tools.h"
#define TEST_IMG "test.jpg"
#define IN_BUF_SIZE 4096

char smtp_server[56]="smtp.163.com";
char Content[6000]={0};
char From[128];
char To[128];
char in_buf[IN_BUF_SIZE+ 1] = "\0";
int g_sock;



void send_socket(const char *out_buf, int size)
{
    int count = 0;
    int ret = 0;
    int len = 0;

    count = 0;
    len = (-1 == size) ? strlen(out_buf) : size;
    while (count < len)
    {
        ret = write(g_sock, out_buf + count, len - count);
        if (ret <= 0)
        {
            perror("write");
            exit(-1);
        }
        count += ret;
    }
}


int sendmail(char* name, char* passwd, char* from, char* to, char* subject, char* content) {
  FILE* file = fopen("email.log", "a");
  FILE * fp = NULL;
  long length = 0L;
  long count = 0L;
  g_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == g_sock)
    {
        perror("socket");
        return -1;
    }
	//1.连接主机服务器
	
	if(connectHost(smtp_server, 25) < 0) {
    return -1;
	}
  //2.判断登录
  if(login(name, passwd) < 0) {
    return -1;
  }

  //写入from
  if(from == "" || to == "" || subject == "" || content == "") {
    printf("arguments error!\n");
    return -1;
  }

  sprintf(From, "MAIL FROM: <%s>\r\n", from);

  if((ret = send(sockfd, From, strlen(From), 0)) == SOCKET_ERROR) {
    return -1;
  }

  if(getResponse() < 0){
    return -1;
  }

  //写入to
  sprintf(To, "RCPT TO: <%s>\r\n", to);
  ret = send(sockfd, To, strlen(To),0);
  fprintf(file, "%s %s - send TO: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send TO");
    return -1;
  }
  if(getResponse() < 0) {
    return -1;
  }
  //写入时间
  send_data = "DATA\r\n";
  ret = send(sockfd,send_data,strlen(send_data),0);
  fprintf(file, "%s %s - send DATA: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send DATA");
    return -1;
  }
  if(getResponse() < 0) {
    return -1;
  }
  
  sprintf(Content,"from:%s\nto:%s\nsubject:%s\n%s\r\n.\r\n",from,to,subject,content);
  ret= send(sockfd, Content, strlen(Content), 0);
  
  fprintf(file, "%s %s - send CONTENT: %s\n", currentTime(), __func__, strerror(errno));
  
  if(ret == SOCKET_ERROR) {
    perror("send CONTENT");
    return -1;
  }
  //写入内容
  //fp为文件句柄
  fp = fopen(TEST_IMG, "rb");
  if(fp == NULL){
      perror("fopen(): ");
      return -1;
  }else{
    printf("获得图片！");
  }

  fseek(fp, 0L, SEEK_END);
  length = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  
  while (count < length){
      ret = fread(in_buf, 1, IN_BUF_SIZE, fp);
      if (ret <= 0){
          perror("fread");
          return -1;
      }
      send_socket(in_buf, ret);
      count += ret;
  }
  fclose(fp);
  send_socket("\r\n\r\n", -1);

  send_socket("--#BOUNDARY#\r\n", -1);
  send_socket("CONTENT-TYPE: APPLICATION/MSWORD; NAME=", -1); /* msword为word类型的 */
  send_socket(TEST_IMG, -1);
  send_socket("\r\n", -1);
  send_socket("CONTENT-DISPOSITION: ATTACHMENT; FILENAME=", -1); /* attachment表示只有当用户点击时文档才会播放(即预览) */
  send_socket(TEST_IMG, -1);
  send_socket("\r\n", -1);
  send_socket("CONTENT-TRANSFER-ENCODING:BASE64\r\n\r\n", -1); /* 附件传输要用base64编码 */




  //设置内存一块存放buffer
  memset(buffer, '\0', sizeof(buffer));

  if(getResponse() < 0) {
    return -1;
  }

  ret = send(sockfd,"QUIT\r\n",strlen("QUIT\r\n"), 0);
  fprintf(file, "%s %s - send QUIT: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send QUIT");
    return -1;
  }
  if(getResponse() < 0) {
    return -1;
  }



  printf("发送成功！\n");
  fclose(file);
  return 0;
}

void send_mail(char* to){
  //获取标题
  printf("Subject: ");
  char subject[512];
  fgets(subject,512,stdin);

  //获取邮件内容
  printf("Content: ");
  char c;
  int p = 0;
  char content[6000];
  while ((c = getchar()) != EOF) {
      content[p++] = c;
  }
  content[p] = '\0';

  char name[100];
  char passwd[100];
  getNamePasswd(name,passwd);
  char* from = base64_decode(name);
  sendmail(name,passwd,from, to, subject, content);
}
#endif
