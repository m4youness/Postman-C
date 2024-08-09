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
        return strdup("");
    }

    size_t path_len = strlen(path_start);
    char *path = (char *)malloc(path_len + 1);
    if (path == NULL)
    {
        return NULL;
    }

    strcpy(path, path_start);

    return path;
}

int getPort(const char *url)
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

    return -1;
}

void to_lowercase(char *str)
{
    while (*str)
    {
        *str = tolower((unsigned char)*str);
        str++;
    }
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