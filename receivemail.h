#ifndef _RECEIVEMAIL_H_ 
#define _RECEIVEMAIL_H_
#include "tools.h"

char imap_server[56]="imap.163.com";
int imap_port = 993;

int watch_inbox() {
  FILE* file = fopen("email.log", "a");

  sslConnect(imap_server, imap_port);
  getResponse();

  char ch[1024];
	char name[100];
  char passwd[100];
  if(getNamePasswd(name, passwd) < 0) {
    exit(EXIT_FAILURE);
  }

  sprintf(ch, "a001 LOGIN %s %s\r\n", base64_decode(name), base64_decode(passwd));
  ret = SSL_write(ssl, (char *)ch, strlen(ch));
  fprintf(file, "%s %s - send LOGIN: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send LOGIN");
    exit(EXIT_FAILURE);
  }

  if(getResponse() < 0) {
    exit(EXIT_FAILURE);
  }

  sprintf(ch, "a002 SELECT INBOX\r\n");
  ret = SSL_write(ssl, (char *)ch, strlen(ch));
  fprintf(file, "%s %s - send SELECT INBOX: %s\n", currentTime(), __func__, strerror(errno));
  if(ret == SOCKET_ERROR) {
    perror("send SELECT INBOX");
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
    printf("Inbox is empty.\n");
    return -1;
  }

  int i;
  for (i = 0; i < total; ++i) {
    sprintf(ch, "a003 fetch %d FLAGS\r\n", i + 1);
    ret = SSL_write(ssl, (char *)ch, strlen(ch));
//    printf("ch - %s\n", ch);
    fprintf(file, "%s %s - send FETCH FLAGS: %s\n", currentTime(), __func__, strerror(errno));
    if (ret == SOCKET_ERROR) {
      perror("send FETCH FLAGS");
      exit(EXIT_FAILURE);
    }
    if(getResponse() < 0) {
      exit(EXIT_FAILURE);
    }
    char* q;
    strtok(buffer,"(");
    strtok(NULL,"(");
    q = strtok(NULL,")");
    if(strcmp(q, "\\Seen") != 0) q = "";
    printf("Email %d: (%s):\n", i + 1, q);

    sprintf(ch, "a003 fetch %d BODY[HEADER.FIELDS (DATE FROM TO SUBJECT)]\r\n", i + 1);
    ret = SSL_write(ssl, (char *)ch, strlen(ch));
//    printf("ch - %s\n", ch);
    fprintf(file, "%s %s - send FETCH BODY HEADER: %s\n", currentTime(), __func__, strerror(errno));
    if (ret == SOCKET_ERROR) {
      perror("send FETCH BODY HEADER");
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
    ret = SSL_write(ssl, (char *)ch, strlen(ch));
    fprintf(file, "%s %s - send FETCH BODY: %s\n", currentTime(), __func__, strerror(errno));
    if (ret == SOCKET_ERROR) {
      perror("send FETCH BODY");
      exit(EXIT_FAILURE);
    }
    if(getResponse() < 0) {
      exit(EXIT_FAILURE);
    }
    strtok(buffer,"\n");
    p = strtok(NULL,"\n");
    printf("Content:\n%s\n",p);
//    printf("Content:\n%s\n",base64_decode(p));
    printf("-------------------------------------\n");
  }

	printf("Delete a message?[y/n]\n");
	char del;
	scanf("%c",&del);
	if(del == 'y') {
		printf("Input the NO. to delete:");
		int delete_i;
		scanf("%d", &delete_i);
		while(delete_i > total) {
			printf("Input error.\n");
			printf("Input the NO. to delete:");
			scanf("%d",&delete_i);
		}
		sprintf(ch, "a004 STORE %d +flags (\\Deleted)\r\n",delete_i);
    fprintf(file, "%s %s - send DELETE FLAG: %s\n", currentTime(), __func__, strerror(errno));
    if(SSL_write(ssl, (char *)ch, strlen(ch)) == SOCKET_ERROR) {
		  perror("send DELETE FLAG");
			exit(EXIT_FAILURE);
		}
		if(getResponse() < 0) {
      exit(EXIT_FAILURE);
		}
		printf("Message deleted！\n");
	}
	return 0;
}

#endif
