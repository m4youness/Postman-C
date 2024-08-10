#include "utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void exit_program()
{
    system("clear");
    exit(EXIT_SUCCESS);
}

void connect_to_socket(char **URL, char **Path, int *Sock_fd)
{
    char url_buffer[255];
    printf("Enter URL or paste text:\n");
    if (fgets(url_buffer, sizeof(url_buffer), stdin) != NULL)
    {
        size_t len = strlen(url_buffer);
        if (len > 0 && url_buffer[len - 1] == '\n')
        {
            url_buffer[len - 1] = '\0';
        }
    }

    *URL = strdup(url_buffer);

    int port = getPort(*URL);
    if (port == -1)
    {
        port = 80;
    }
    char port_str[10];
    snprintf(port_str, sizeof(port_str), "%d", port);

    char *path = getPath(*URL);
    char *lower_case_path = strdup(path);
    to_lowercase(lower_case_path);

    remove_substring(*URL, "https://");
    remove_substring(*URL, "http://");
    remove_substring(*URL, lower_case_path);
    remove_substring(*URL, port_str);
    remove_substring(*URL, ":");

    int sock_fd, status;
    struct addrinfo hints, *addr, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(*URL, port_str, &hints, &addr)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        free(*URL);
        free(lower_case_path);
        return;
    }

    for (p = addr; p != NULL; p = p->ai_next)
    {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype, 0)) != -1)
        {
            if (connect(sock_fd, p->ai_addr, p->ai_addrlen) != -1)
            {
                break;
            }
        }
        close(sock_fd);
    }

    if (p == NULL)
    {
        perror("socket or connect");
        free(*URL);
        free(lower_case_path);
        freeaddrinfo(addr);
        return;
    }

    freeaddrinfo(addr);

    *Path = strdup(path);
    *Sock_fd = sock_fd;
    free(lower_case_path);
}

void get_func()
{
    char *Url = NULL;
    char *path = NULL;
    int sock_fd = -1;

    connect_to_socket(&Url, &path, &sock_fd);

    if (Url && path && sock_fd != -1)
    {

        char request[BUFFER_SIZE];
        snprintf(request, sizeof(request),
                 "GET %s HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 path, Url);

        send(sock_fd, request, strlen(request), 0);

        int n;
        char response[BUFFER_SIZE];

        while ((n = recv(sock_fd, response, sizeof(response) - 1, 0)) > 0)
        {
            response[n] = '\0';
            printf("Bytes read: %d\n", n);
            printf("Data: %s\n", response);
        }

        if (n < 0)
        {
            perror("read");
        }

        close(sock_fd);
    }

    free(Url);
    free(path);
}

void post_func()
{
    char *Url = NULL;
    char *Path = NULL;
    int sock_fd = -1;

    printf("Add any parameters in body.json\n");

    connect_to_socket(&Url, &Path, &sock_fd);

    long content_length = 0;
    char *content = NULL;

    open_file(&content_length, &content);

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "POST %s HTTP/1.1\r\n"
                                       "Host: %s\r\n"
                                       "Content-Type: application/json\r\n"
                                       "Content-Length: %ld\r\n"
                                       "Connection: close\r\n"
                                       "\r\n"
                                       "%s",
             Path, Url, content_length, content);

    send(sock_fd, request, sizeof(request), 0);

    char response[BUFFER_SIZE];
    int n;

    while ((n = recv(sock_fd, response, sizeof(response) - 1, 0)) > 0)
    {
        response[n] = '\0';
        printf("Bytes read: %d\n", n);
        printf("Data: %s\n", response);
    }

    if (n < 0)
    {
        perror("recv");
        close(sock_fd);
        free(Url);
        free(Path);
        free(content);
        return;
    }

    free(Url);
    free(Path);
    free(content);
    close(sock_fd);
}

void put_func() { printf("I'm a PUT func\n"); }

void delete_func() { printf("I'm a DELETE func\n"); }

void choose_option(int option)
{
    if (option > 0 && option <= 5)
    {
        switch (option)
        {
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
    }
    else
    {
        fprintf(stderr, "Please enter a correct value!\n");
    }
}

void display_menu()
{
    int option = 0;

    printf("\n--------------------- HTTP Client Menu ---------------------\n\n");
    printf("\t\t1. Send GET request\n");
    printf("\t\t2. Send POST request\n");
    printf("\t\t3. Send PUT request\n");
    printf("\t\t4. Send DELETE request\n");
    printf("\t\t5. Exit\n");

    printf("Please choose an option: ");
    if (scanf("%d", &option) != 1)
    {
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

int main()
{
    display_menu();
    return 0;
}
