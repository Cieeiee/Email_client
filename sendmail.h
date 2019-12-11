#ifndef _SENDMAIL_H_ 
#define _SENDMAIL_H_
#include "tools.h"

char smtp_server[56]="smtp.163.com";
char Content[6000]={0};
char From[128];
char To[128];

int sendmail(char* name, char* passwd, char* from, char* to, char* subject, char* content) {
  FILE* file = fopen("email.log", "a");
	//1.连接主机服务器
	//2.登录
	//3.发送邮件
	if(connectHost(smtp_server, 25) < 0) {
    return -1;
	}
 
  if(login(name, passwd) < 0) {
    return -1;
  }

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
