#include "utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void exit_program() {
  system("clear");
  exit(EXIT_SUCCESS);
}

void init_openssl() {
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
}

SSL_CTX *create_ssl_context() {
  const SSL_METHOD *method;
  SSL_CTX *ctx;

  method = TLS_client_method();
  ctx = SSL_CTX_new(method);
  if (!ctx) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  return ctx;
}

SSL *create_ssl_connection(SSL_CTX *ctx, int sockfd) {
  SSL *ssl;

  ssl = SSL_new(ctx);
  SSL_set_fd(ssl, sockfd);

  if (SSL_connect(ssl) <= 0) {
    ERR_print_errors_fp(stderr);
    SSL_free(ssl);
    exit(EXIT_FAILURE);
  }

  return ssl;
}

void send_data(SSL *ssl, const char *data) {
  if (SSL_write(ssl, data, strlen(data)) <= 0) {
    ERR_print_errors_fp(stderr);
  }
}

void receive_data(SSL *ssl) {
  char buffer[1024];
  int bytes;

  while ((bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1)) > 0) {
    buffer[bytes] = '\0';
    printf("Data: %s\n", buffer);
    printf("Bytes read: %d\n", bytes);
  }

  if (bytes < 0) {
    ERR_print_errors_fp(stderr);
  }
}

void cleanup(SSL *ssl, SSL_CTX *ctx) {
  SSL_shutdown(ssl);
  SSL_free(ssl);
  SSL_CTX_free(ctx);
}

void create_socket(char **URL, char **Path, int *Sock_fd, int *Port) {
  char url_buffer[255];
  printf("Enter URL or paste text:\n");

  // Read user input
  if (fgets(url_buffer, sizeof(url_buffer), stdin) != NULL) {
    size_t len = strlen(url_buffer);
    if (len > 0 && url_buffer[len - 1] == '\n') {
      url_buffer[len - 1] = '\0';
    }
  }

  if (url_buffer[0] == '\0') {
    fprintf(stderr, "Error: No URL provided.\n");
    return;
  }

  *URL = strdup(url_buffer);
  if (*URL == NULL) {
    perror("strdup");
    return;
  }

  char *path = getPath(*URL);
  if (path == NULL) {
    fprintf(stderr, "Error: Failed to extract path from URL.\n");
    free(*URL);
    *URL = NULL;
    return;
  }

  char *lower_case_path = strdup(path);
  if (lower_case_path == NULL) {
    perror("strdup");
    free(*URL);
    *URL = NULL;
    free(path);
    return;
  }
  to_lowercase(lower_case_path);

  int port = getPort(*URL);

  *Port = port;

  char port_str[10];
  snprintf(port_str, sizeof(port_str), "%d", port);

  remove_substring(*URL, "https://");
  remove_substring(*URL, "http://");
  remove_substring(*URL, lower_case_path);
  remove_substring(*URL, port_str);
  remove_substring(*URL, ":");
  free(lower_case_path);

  if (**URL == '\0') {
    fprintf(stderr, "Error: Invalid URL after processing.\n");
    free(*URL);
    *URL = NULL;
    free(path);
    return;
  }

  int sock_fd, status;
  struct addrinfo hints, *addr, *p;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((status = getaddrinfo(*URL, port_str, &hints, &addr)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    free(*URL);
    *URL = NULL;
    free(path);
    return;
  }

  for (p = addr; p != NULL; p = p->ai_next) {
    if ((sock_fd = socket(p->ai_family, p->ai_socktype, 0)) == -1) {
      continue;
    }

    if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sock_fd);
      continue;
    }

    break;
  }

  freeaddrinfo(addr);

  if (p == NULL) {
    perror("socket or connect");
    free(*URL);
    *URL = NULL;
    free(path);
    return;
  }

  *Path = strdup(path);
  if (*Path == NULL) {
    perror("strdup");
    close(sock_fd);
    free(*URL);
    *URL = NULL;
    free(path);
    return;
  }

  *Sock_fd = sock_fd;
  free(path);
}

void send_request(char *Url, char *path, int sock_fd, int Port,
                  char request[1024]) {

  if (Url && path && sock_fd != -1 && Port != -1) {

    if (Port == 443) {
      init_openssl();
      SSL_CTX *ctx = create_ssl_context();
      SSL *ssl = create_ssl_connection(ctx, sock_fd);

      send_data(ssl, request);
      receive_data(ssl);

      cleanup(ssl, ctx);
    } else {
      send(sock_fd, request, strlen(request), 0);

      int n;
      char response[BUFFER_SIZE];

      while ((n = recv(sock_fd, response, sizeof(response) - 1, 0)) > 0) {
        response[n] = '\0';
        printf("Data: %s\n", response);
        printf("Bytes read: %d\n", n);
      }

      if (n < 0) {
        perror("read");
      }
    }
    close(sock_fd);
  }

  free(Url);
  free(path);
}

void get_func() {
  char *Url = NULL;
  char *path = NULL;
  int sock_fd = -1;
  int Port = -1;

  create_socket(&Url, &path, &sock_fd, &Port);

  char request[BUFFER_SIZE];
  snprintf(request, sizeof(request),
           "GET %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n"
           "\r\n",
           path, Url);

  send_request(Url, path, sock_fd, Port, request);
}

void post_func() {
  char *Url = NULL;
  char *Path = NULL;
  int sock_fd = -1;
  int port = -1;

  printf("Add any parameters in body.json\n");

  create_socket(&Url, &Path, &sock_fd, &port);
  long content_length = 0;
  char *content = NULL;

  open_file(&content_length, &content);
  char request[BUFFER_SIZE];
  snprintf(request, sizeof(request),
           "POST %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: %ld\r\n"
           "Connection: close\r\n"
           "\r\n"
           "%s",
           Path, Url, content_length, content);

  send_request(Url, Path, sock_fd, port, request);
}

void put_func() {
  char *Url = NULL;
  char *Path = NULL;
  int sock_fd = -1;
  int port = -1;

  printf("Add any parameters in body.json\n");

  create_socket(&Url, &Path, &sock_fd, &port);
  long content_length = 0;
  char *content = NULL;

  open_file(&content_length, &content);
  char request[BUFFER_SIZE];
  snprintf(request, sizeof(request),
           "PUT %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: %ld\r\n"
           "Connection: close\r\n"
           "\r\n"
           "%s",
           Path, Url, content_length, content);

  send_request(Url, Path, sock_fd, port, request);
}

void delete_func() {
  char *Url = NULL;
  char *path = NULL;
  int sock_fd = -1;
  int Port = -1;

  create_socket(&Url, &path, &sock_fd, &Port);

  char request[BUFFER_SIZE];
  snprintf(request, sizeof(request),
           "DELETE %s HTTP/1.1\r\n"
           "Host: %s\r\n"
           "Connection: close\r\n"
           "\r\n",
           path, Url);

  send_request(Url, path, sock_fd, Port, request);
}

void choose_option(int option) {
  if (option > 0 && option <= 5) {
    switch (option) {
    case 1:
      get_func();
      break;
    case 2:
      post_func();
      break;
    case 3:
      put_func();
      break;
    case 4:
      delete_func();
      break;
    case 5:
      exit_program();
    }
  } else {
    fprintf(stderr, "Please enter a correct value!\n");
  }
}

void display_menu() {
  int option = 0;

  printf("\n--------------------- HTTP Client Menu ---------------------\n\n");
  printf("\t\t1. Send GET request\n");
  printf("\t\t2. Send POST request\n");
  printf("\t\t3. Send PUT request\n");
  printf("\t\t4. Send DELETE request\n");
  printf("\t\t5. Exit\n");

  printf("\nPlease choose an option: ");
  if (scanf("%d", &option) != 1) {
    fprintf(stderr, "Invalid input. Please enter a number.\n");
    while (getchar() != '\n')
      ;
    return;
  }

  while (getchar() != '\n')
    ;

  system("clear");
  choose_option(option);
}

int main() {
  display_menu();
  return 0;
}
