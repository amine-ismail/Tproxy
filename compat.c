#include <stdio.h>
#include <float.h>
#include <stdlib.h>

int __strtorQ(const char*, char**, int, void*);

long double strtold(const char* s, char** end_ptr) {
#if __LP64__
  long double result;
  __strtorQ(s, end_ptr, FLT_ROUNDS, &result);
  return result;
#else
  // This is fine for LP32 where long double is just double.
  return strtod(s, end_ptr);
#endif
}

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
    char *ptr;
    ptr = fgetln(stream, n);
    if (ptr == NULL) {
        return -1;
    }
    /* Free the original ptr */
    if (*lineptr != NULL) free(*lineptr);
    /* Add one more space for '\0' */
    size_t len = n[0] + 1;
    /* Update the length */
    n[0] = len;
    /* Allocate a new buffer */
    *lineptr = malloc(len);
    /* Copy over the string */
    memcpy(*lineptr, ptr, len-1);
    /* Write the NULL character */
    (*lineptr)[len-1] = '\0';
    /* Return the length of the new buffer */
    return len;
}

char *strchrnul(const char *s, int c)
{
   char * matched_char = strchr(s, c);

   if (matched_char == NULL) {
       matched_char = (char*) s + strlen(s);
   }

   return matched_char;
}
