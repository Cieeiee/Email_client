#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#ifndef NETWORK_SSLCONNECT_H
#define NETWORK_SSLCONNECT_H

SSL_CTX *ctx;
SSL *ssl;
X509 *cert = NULL;
char buffer[1024];

int create_socket(char[], int, BIO *);

int sslConnect(char *dest_url, int port) {
  BIO *certbio = NULL;
  BIO *outbio = NULL;
  X509_NAME *certname = NULL;
  const SSL_METHOD *method;
  int server = 0;
  int ret, i;

  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();

  certbio = BIO_new(BIO_s_file());
  outbio = BIO_new_fp(stdout, BIO_NOCLOSE);

  if(SSL_library_init() < 0)
    BIO_printf(outbio, "Could not initialize the OpenSSL library.\n");
  method = SSLv23_client_method();
  if((ctx = SSL_CTX_new(method)) == NULL)
    BIO_printf(outbio, "Unable to create a new SSL context structure.\n");
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);

  ssl = SSL_new(ctx);
  server = create_socket(dest_url, port, outbio);
  if(server != 0)
    BIO_printf(outbio, "Successfully made the TCP connection to: %s.\n", dest_url);
  SSL_set_fd(ssl, server);
  if(SSL_connect(ssl) != 1)
    BIO_printf(outbio, "Error: Could not build a SSL session to: %s.\n", dest_url);
  else
    BIO_printf(outbio, "Successfully enabled SSL/TLS session to: %s.\n", dest_url);

  cert = SSL_get_peer_certificate(ssl);
  if(cert == NULL)
    BIO_printf(outbio, "Error: Could not get a certificate from: %s.\n", dest_url);
  else
    BIO_printf(outbio, "Retrieved the server's certificate from: %s.\n", dest_url);
  certname = X509_NAME_new();
  certname = X509_get_subject_name(cert);
  BIO_printf(outbio, "Displaying the certificate subject data:\n");
  X509_NAME_print_ex(outbio, certname, 0, 0);
  BIO_printf(outbio, "\n");
}

char* get_ipv6_addr(char* hostname) {
  static char buf[500];
  struct addrinfo *host;
  struct sockaddr_in6 *ipv6;
  getaddrinfo(hostname, NULL, NULL, &host);
  while (host->ai_family != AF_INET6 && host->ai_next != NULL)
    host = host->ai_next;
  ipv6 = (struct sockaddr_in6*)host->ai_addr;
  inet_ntop(host->ai_family, &ipv6->sin6_addr, buf, sizeof(buf));
  return buf;
}

char* get_ipv4_addr(char* hostname) {
  static char buf[500];
  struct addrinfo *host;
  struct sockaddr_in *ipv4;
  getaddrinfo(hostname, NULL, NULL, &host);
  while (host->ai_family != AF_INET && host->ai_next != NULL)
    host = host->ai_next;
  ipv4 = (struct sockaddr_in*)host->ai_addr;
  inet_ntop(host->ai_family, &ipv4->sin_addr, buf, sizeof(buf));
  return buf;
}

int create_socket(char* hostname, int port, BIO *out) {
  int sockfd, sockfd6;
  struct hostent *host;
  struct sockaddr_in dest_addr;
  struct sockaddr_in6 dest_addr6;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  sockfd6 = socket(AF_INET6, SOCK_STREAM, 0);

  bzero(&dest_addr, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);

  bzero(&dest_addr6, sizeof(dest_addr6));
  dest_addr6.sin6_family = AF_INET6;
  dest_addr6.sin6_port = htons(port);
  inet_pton(AF_INET, get_ipv4_addr(hostname), &dest_addr.sin_addr);
  inet_pton(AF_INET6, get_ipv6_addr(hostname), &dest_addr6.sin6_addr);

  if(connect(sockfd6, (struct sockaddr*)&dest_addr6, sizeof(dest_addr6)) < 0) {
    perror("IPv6 connect");
    printf("try IPv4...\n");
    if(connect(sockfd, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
      perror("IPv4 connect");
      exit(EXIT_FAILURE);
    }
    return sockfd;
  }

  return sockfd6;
}

#endif //NETWORK_SSLCONNECT_H
