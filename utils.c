#include "utils.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void itoa(int val, char *str, int base)
{
    val = (uint16_t)val;
    if (val == 0)
    {
        *str++ = '0';
        *str = '\0';
        return;
    }

    int i = 0;
    char temp[10];

    while (val)
    {
        temp[i++] = "0123456789"[val % base];
        val /= base;
    }

    while (i)
    {
        *str++ = temp[--i];
    }

    *str = '\0';
}

char *getPath(const char *url)
{
    const char *scheme_end = strstr(url, "://");
    if (scheme_end == NULL)
    {
        scheme_end = url;
    }
    else
    {
        scheme_end += 3;
    }

    const char *path_start = strchr(scheme_end, '/');
    if (path_start == NULL)
    {
        return strdup("/");
    }

    size_t path_len = strlen(path_start);
    char *path = (char *)malloc(path_len + 1);
    if (path == NULL)
    {
        perror("Malloc");
        return NULL;
    }
    strcpy(path, path_start);

    return path;
}

int getPort(const char *url)
{
    if (strncmp(url, "https://", 8) == 0)
    {
        return 443;
    }

    if (strncmp(url, "http://", 7) == 0)
    {
        return 80;
    }

    const char *scheme_end = strstr(url, "://");
    if (scheme_end == NULL)
    {
        scheme_end = url;
    }
    else
    {
        scheme_end += 3;
    }

    const char *path_start = strchr(scheme_end, '/');
    if (path_start == NULL)
    {
        path_start = scheme_end + strlen(scheme_end);
    }

    const char *colon_pos = strchr(scheme_end, ':');
    if (colon_pos != NULL && colon_pos < path_start)
    {
        colon_pos++;

        char port_str[6];
        const char *port_end = strchr(colon_pos, '/');
        if (port_end == NULL)
        {
            port_end = scheme_end + strlen(scheme_end);
        }

        size_t port_len = port_end - colon_pos;
        if (port_len >= sizeof(port_str))
        {
            port_len = sizeof(port_str) - 1;
        }

        strncpy(port_str, colon_pos, port_len);
        port_str[port_len] = '\0';

        return atoi(port_str);
    }

    return 80;
}

void to_lowercase(char *str)
{
    while (*str)
    {
        *str = tolower((unsigned char)*str);
        str++;
    }
}

void open_file(long *content_length, char **content)
{
    FILE *fptr = fopen("body.json", "r");
    if (fptr == NULL)
    {
        perror("Error opening file");
        return;
    }

    fseek(fptr, 0, SEEK_END);

    long file_size = ftell(fptr);

    fseek(fptr, 0, SEEK_SET);

    char *buf = (char *)malloc(file_size + 1);
    if (buf == NULL)
    {
        perror("Malloc");
        return;
    }
    fread(buf, 1, file_size, fptr);

    *content = strdup(buf);
    *content_length = file_size;

    fclose(fptr);
    free(buf);
}

void remove_substring(char *str, const char *sub)
{
    to_lowercase(str);
    char *pos;
    size_t sub_len;

    while ((pos = strstr(str, sub)) != NULL)
    {
        sub_len = strlen(sub);
        memmove(pos, pos + sub_len, strlen(pos + sub_len) + 1);
    }
}