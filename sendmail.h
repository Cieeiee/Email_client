#ifndef _SENDMAIL_H_ 
#define _SENDMAIL_H_
#include "tools.h"

char smtp_server[56]="smtp.163.com";
int smtp_port = 587;
char Content[6000]={0};
char From[128];
char To[128];

int sendmail(char* name, char* passwd, char* from, char* to, char* subject, char* content) {
  FILE* file = fopen("email.log", "a");

	sslConnect(smtp_server, smtp_port);
  getResponse();

  if(login(name, passwd) < 0) {
    return -1;
  }

  if(from == "" || to == "" || subject == "" || content == "") {
    printf("arguments error!\n");
    return -1;
  }

  sprintf(From, "MAIL FROM: <%s>\r\n", from);

  if((ret = SSL_write(ssl, From, strlen(From))) == SOCKET_ERROR) {
    return -1;
  }

  if(getResponse() < 0) {
    return -1;
  }

  sprintf(To, "RCPT TO: <%s>\r\n", to);
  ret = SSL_write(ssl, To, strlen(To));
  fprintf(file, "%s %s - send TO: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send TO");
    return -1;
  }
  if(getResponse() < 0) {
    return -1;
  }

  send_data = "DATA\r\n";
  ret = SSL_write(ssl, send_data, strlen(send_data));
  fprintf(file, "%s %s - send DATA: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send DATA");
    return -1;
  }
  if(getResponse() < 0) {
    return -1;
  }

  sprintf(Content,"from:%s\nto:%s\nsubject:%s\n%s\r\n.\r\n",from,to,subject,content);
  ret = SSL_write(ssl, Content, strlen(Content));
  fprintf(file, "%s %s - send CONTENT: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send CONTENT");
    return -1;
  }

  memset(buffer, '\0', sizeof(buffer));

  if(getResponse() < 0) {
    return -1;
  }

  ret = SSL_write(ssl, "QUIT\r\n",strlen("QUIT\r\n"));
  fprintf(file, "%s %s - send QUIT: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send QUIT");
    return -1;
  }
  if(getResponse() < 0) {
    return -1;
  }

  printf("Message sent.\n");
  fclose(file);
  return 0;
}

void send_mail(char* to){

  printf("Subject: ");
  char subject[512];
  fgets(subject,512,stdin);

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
