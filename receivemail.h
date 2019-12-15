#ifndef _RECEIVEMAIL_H_ 
#define _RECEIVEMAIL_H_
#include "tools.h"

char imap_server[56]="imap.163.com";

int watch_inbox() {
  printf("进入收件箱\n");
  FILE* file = fopen("email.log", "a");
	//连接服务器,参数，服务器地址，ip地址，端口号
	if(connectHost(imap_server, 143) < 0) {
	  exit(EXIT_FAILURE);
	}
  //登录
  char ch[1024];
	char name[100];
  char passwd[100];
  if(getNamePasswd(name, passwd) < 0) {
    exit(EXIT_FAILURE);
  }

  //##########日志处理开始
  //格式化输出到ch中（a001 LOGIN 13473995461@163.com wyyx118329）
  sprintf(ch, "a001 LOGIN %s %s\r\n", base64_decode(name), base64_decode(passwd));
  //功能是向一个已经连接的socket发送数据，如果无错误，返回值为所发送数据的总数，否则返回SOCKET_ERROR。
  ret = send(sockfd, (char *)ch, strlen(ch),0);
  //fprintf：将当前时间、功能？错误类型写到file文件中，错误处理
  fprintf(file, "%s %s - send LOGIN: %s\n", currentTime(), __func__, strerror(errno));
  //发送数据失败
  if(ret == SOCKET_ERROR) {
    perror("send LOGIN");
    exit(EXIT_FAILURE);
  }
  if(getResponse() < 0) {
    exit(EXIT_FAILURE);
  }
  //##########日志处理结束


  //##########日志处理开始
  sprintf(ch, "a002 SELECT INBOX\r\n");
  ret = send(sockfd, (char *)ch, strlen(ch),0);
  fprintf(file, "%s %s - send SELECT INBOX: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send SELECT INBOX");
    exit(EXIT_FAILURE);
  }
  if(getResponse() < 0) {
    exit(EXIT_FAILURE);
  }
  //##########日志处理结束


  char* p;
  //按照\n分解字符串buffer
  strtok(buffer," \n");
  //printf("log：buffer为：%s\n", buffer);
  p = strtok(NULL," \n");
  //printf("log：p为%s\n", p);
  //p已经是邮件的个数了，这里是转化一下
  long int total = strtol(p, NULL, 10);
  //printf("log:total为%ld\n", total);
  if(total == 0) {
    printf("邮箱为空！\n");
    return -1;
  }


  int i;
  for (i = 0; i < total; ++i) {
    //##########日志处理开始
    sprintf(ch, "a003 fetch %d FLAGS(SEEN)\r\n", i + 1);
    ret = send(sockfd, (char *)ch, strlen(ch), 0);
    fprintf(file, "%s %s - send FETCH FLAGS: %s\n", currentTime(), __func__, strerror(errno));
    if (ret == SOCKET_ERROR) {
      perror("send FETCH FLAGS");
      exit(EXIT_FAILURE);
    }
    if(getResponse() < 0) {
      exit(EXIT_FAILURE);
    }
    //##########日志处理结束
    p = strtok(buffer,"\n");
    char* q;
    strtok(p," ");
    strtok(NULL," ");
    strtok(NULL," ");
    strtok(NULL," ");
    q = strtok(NULL," ");
    q = strtok(q,")");

    //printf("log:q为%s：\n", q);
    printf("第%d封邮件 %s):\n", i + 1, q);

    //##########日志处理开始
    sprintf(ch, "a003 fetch %d BODY[HEADER.FIELDS (DATE FROM TO SUBJECT)]\r\n",i+1);
    ret = send(sockfd, (char *)ch, strlen(ch),0);
    fprintf(file, "%s %s - send FETCH BODY HEADER: %s\n", currentTime(), __func__, strerror(errno));
    if (ret == SOCKET_ERROR) {
      perror("send FETCH BODY HEADER");
      exit(EXIT_FAILURE);
    }
    if(getResponse() < 0) {
      exit(EXIT_FAILURE);
    }
    
    strtok(buffer,"\n");
    //printf("hhhhhp:%s\n", p);
    //##########日志处理结束  
    q = strtok(NULL,"\n");
    printf("%s\n",q);
    q = strtok(NULL,"\n");
    printf("%s\n",q);
    q = strtok(NULL,"\n");
    printf("%s\n",q);
    q = strtok(NULL,"\n");
    printf("%s\n",q);


    //##########日志处理开始
    sprintf(ch, "a003 fetch %d BODY[1]\r\n", i + 1);
    ret = send(sockfd, (char *)ch, strlen(ch),0);
    fprintf(file, "%s %s - send FETCH BODY: %s\n", currentTime(), __func__, strerror(errno));
    if (ret == SOCKET_ERROR) {
      perror("send FETCH BODY");
      exit(EXIT_FAILURE);
    }
    if(getResponse() < 0) {
      exit(EXIT_FAILURE);
    }
    //##########日志处理结束
    strtok(buffer,"\n");
    p = strtok(NULL,"\n");
    printf("内容:\n%s\n",base64_decode(p));
    //printf("Content:\n%s\n",*base64_decode((const char *)p));
    printf("-------------------------------------\n");
  }


	printf("是否删除邮件？[y/n]\n");
	char del;
	scanf("%c",&del);
	if(del == 'y') {
		printf("请输入要删除邮件的编号:");
		int delete_i;
		scanf("%d", &delete_i);
		while(delete_i > total) {
			printf("输入有误，请重新输入。\n");
			printf("请输入要删除邮件的编号:");
			scanf("%d",&delete_i);
		}
    //##########日志处理开始
		sprintf(ch, "a004 STORE %d +flags (\\Deleted)\r\n",delete_i);
    fprintf(file, "%s %s - send DELETE FLAG: %s\n", currentTime(), __func__, strerror(errno));
		//##########日志处理结束
    if (send(sockfd, (char *)ch, strlen(ch),0) == SOCKET_ERROR) {
		  perror("send DELETE FLAG");
			exit(EXIT_FAILURE);
		}
		if(getResponse() < 0) {
      exit(EXIT_FAILURE);
		}
		printf("删除成功！\n");
	}
	return 0;
}

#endif
