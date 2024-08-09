#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void exit_program()
{
    system("clear");
    exit(0);
}

void get_func()
{
    char Url[255];
    printf("Enter URL or paste text:\n");
    if (fgets(Url, sizeof(Url), stdin) != NULL)
    {
        size_t len = strlen(Url);
        if (len > 0 && Url[len - 1] == '\n')
        {
            Url[len - 1] = '\0';
        }
    }

    int port = getPort(Url);
    if (port == -1)
    {
        port = 80;
    }
    char port_str[10];
    itoa(port, port_str, 10);

    // make sure to free path
    char *path = getPath(Url);
    char *lower_case_path = strdup(path);
    to_lowercase(lower_case_path);
    printf("%s\n", path);

    remove_substring(Url, "https://");
    remove_substring(Url, "http://");
    remove_substring(Url, lower_case_path);
    remove_substring(Url, port_str);
    remove_substring(Url, ":");

    int sock_fd, status;

    struct addrinfo hints, *addr, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;

    printf("%s\n", Url);
    printf("%s\n", port_str);

    if ((status = getaddrinfo(Url, port_str, &hints, &addr)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
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
        free(path);
        freeaddrinfo(addr);
        return;
    }

    freeaddrinfo(addr);

    char request[BUFFER_SIZE];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\n"
                                       "Host: %s\r\n"
                                       "Connection: close\r\n"
                                       "\r\n",
             path, Url);

    free(path);

    send(sock_fd, request, sizeof(request), 0);

    int n;

    char response[BUFFER_SIZE];

    while ((n = recv(sock_fd, response, sizeof(response), 0)) > 0)
    {
        printf("Bytes read: %d\n", n);
        printf("Data: %s\n", response);
    }

    if (n < 0)
    {
        perror("read");
        return;
    }

    close(sock_fd);
}

void post_func()
{
    printf("I'm a POST func\n");
}

void put_func()
{
    printf("I'm a PUT func\n");
}

void delete_func()
{
    printf("I'm a DELETE func\n");
}

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
