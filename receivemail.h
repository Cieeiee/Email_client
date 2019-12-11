#ifndef _RECEIVEMAIL_H_ 
#define _RECEIVEMAIL_H_
#include "tools.h"

char imap_server[56]="imap.163.com";

int watch_inbox() {
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

  sprintf(ch, "a001 LOGIN %s %s\r\n", base64_decode(name), base64_decode(passwd));
  if(send(sockfd, (char *)ch, strlen(ch),0) == SOCKET_ERROR) {
    perror("login");
    exit(EXIT_FAILURE);
  }

  if(getResponse() < 0) {
    exit(EXIT_FAILURE);
  }

  sprintf(ch, "a002 SELECT INBOX\r\n");
  if(send(sockfd, (char *)ch, strlen(ch),0) == SOCKET_ERROR) {
    perror("select inbox");
    exit(EXIT_FAILURE);
  }

  if(getResponse() < 0) {
    exit(EXIT_FAILURE);
  }

  char* p;
  strtok(buffer," \n");
  p = strtok(NULL," \n");
  long int total = strtol(p, NULL, 10);
  if(total == 0) {
    printf("邮箱为空！\n");
    return -1;
  }

  int i;
  for (i = 0; i < total; ++i) {
    sprintf(ch, "a003 fetch %d FLAGS(SEEN)\r\n", i + 1);
    if (send(sockfd, (char *)ch, strlen(ch), 0) == SOCKET_ERROR) {
      exit(EXIT_FAILURE);
    }
    if(getResponse() < 0) {
      exit(EXIT_FAILURE);
    }
    p = strtok(buffer,"\n");
    char* q;
    strtok(p," ");
    strtok(NULL," ");
    strtok(NULL," ");
    strtok(NULL," ");
    q = strtok(NULL," ");
    q = strtok(q,")");
    printf("第%d封邮件 %s):\n", i + 1, q);

    sprintf(ch, "a003 fetch %d BODY[HEADER.FIELDS (DATE FROM TO SUBJECT)]\r\n",i+1);
    if (send(sockfd, (char *)ch, strlen(ch),0) == SOCKET_ERROR) {
      perror("send");
      exit(EXIT_FAILURE);
    }
    if(getResponse() < 0) {
      exit(EXIT_FAILURE);
    }
    strtok(buffer,"\n");
    q = strtok(NULL,"\n");
    printf("%s\n",q);
    q = strtok(NULL,"\n");
    printf("%s\n",q);
    q = strtok(NULL,"\n");
    printf("%s\n",q);
    q = strtok(NULL,"\n");
    printf("%s\n",q);

    sprintf(ch, "a003 fetch %d BODY[1]\r\n", i + 1);
    if (send(sockfd, (char *)ch, strlen(ch),0) == SOCKET_ERROR) {
      exit(EXIT_FAILURE);
    }
    if(getResponse() < 0) {
      exit(EXIT_FAILURE);
    }
    strtok(buffer,"\n");
    p = strtok(NULL,"\n");
    printf("Content:\n%s\n",p);
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
		sprintf(ch, "a004 STORE %d +flags (\\Deleted)\r\n",delete_i);
		if (send(sockfd, (char *)ch, strlen(ch),0) == SOCKET_ERROR) {
		  perror("send");
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
