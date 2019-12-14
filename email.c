#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "sendmail.h"
#include "receivemail.h"

int main(int argc, char* argv[]) {
  if (argc > 1) {
    if (strcmp(argv[1], "--setuser") == 0)
      setUser();
    else if (strcmp(argv[1], "--send") == 0) {
      char *to = argv[2];
      if (to == NULL) printf("Error: Email address is empty.");
      else send_mail(to);
    }
    else if (strcmp(argv[1], "--inbox") == 0)
      watch_inbox();
    else watch_help();
  }
  else watch_help();
  SSL_free(ssl);
  SSL_CTX_free(ctx);
  X509_free(cert);
  return 0;
}