#ifndef __COMPAT_H__
#define  __COMPAT_H__
long double strtold(const char* s, char** end_ptr);
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
char *strchrnul(const char *s, int c);
#endif
