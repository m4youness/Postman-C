#ifndef MYUTILS_H_
#define MYUTILS_H_

void remove_substring(char *str, const char *sub);
void to_lowercase(char *str);
void open_file(long *content_length, char **content);
int getPort(const char *url);
char *getPath(const char *url);
void itoa(int val, char *str, int base);

#endif